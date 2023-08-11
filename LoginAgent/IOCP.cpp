#include "framework.h"



IOCP::IOCP()
{

}

IOCP::~IOCP()
{
    if (hThreadArr_)
    {
        delete[] hThreadArr_;
    }
}

HRESULT IOCP::Initialize(WORD wPort)
{
    HRESULT hr = S_OK;

    wPort_ = wPort;

    // check system config
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    dwThreadCount_ = systemInfo.dwNumberOfProcessors * 2;

    hThreadArr_ = new HANDLE[dwThreadCount_];

    // WSAStartup
    WSADATA wsaData;
    if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    
    InitializeListHead(&listConnection_);

    InitializeCriticalSection(&csListConn_);


    return hr;
}

HRESULT IOCP::Start()
{
    HRESULT hr = S_OK;
    // create IOCP
    hIOCP_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!hIOCP_)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LOGF(_T("CreateIoCompletionPort failed! hr = %08X"), hr);
        return hr;
    }



    // Start to listen
    hr = StartListen();
    if (FAILED(hr))
    {
        LOGF(_T("StartListen failed! hr = %08X\n"), hr);
        return hr;
    }

    hr = GetExtensionFunctions();
    if (FAILED(hr))
    {
        LOGF(_T("GetExtensionFunctions failed! hr = %08X\n"), hr);
        return hr;
    }


    // create worker thread
    for (DWORD i = 0; i < dwThreadCount_; ++i)
    {
        HANDLE hThread = INVALID_HANDLE_VALUE;
        DWORD dwThreadId = 0;

        hThread = CreateThread(NULL, 0, WorkerThreadProc, this, 0, &dwThreadId);
        if (hThread == NULL) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            LOGF(_T("CreateThread failed! hr = %08X"), hr);
            return hr;

        }
        hThreadArr_[i] = hThread;
        hThread = INVALID_HANDLE_VALUE;
    }
    return hr;
}

HRESULT IOCP::Stop()
{
    HRESULT hr = S_OK;

    //CancelIoEx()

    for (DWORD i = 0; i < dwThreadCount_; ++i)
    {
        if (hThreadArr_[i] != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hThreadArr_[i]);
            hThreadArr_[i] = INVALID_HANDLE_VALUE;
        }
    }

    CloseHandle(hIOCP_);

    return hr;
}


HRESULT IOCP::StartListen()
{
    HRESULT hr = S_OK;

    int nRet = 0;
    int nZero = 0;
    ADDRINFOT hints = { 0 };
    ADDRINFOT* addrlocal = NULL;

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;

    TCHAR value[16];
    StringCchPrintf(value, 16, _T("%d"), wPort_);
    if (GetAddrInfo(NULL, value, &hints, &addrlocal) != 0) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("GetAddrInfo failed! hr = %08X\n"), hr);
        return hr;
    }

    if (addrlocal == NULL) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("GetAddrInfo failed! hr = %08X\n"), hr);
        return hr;
    }


    socketListen_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socketListen_ == INVALID_SOCKET) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("WSASocket failed! hr = %08X\n"), hr);
        return hr;
    }
    if (NULL == CreateIoCompletionPort((HANDLE)socketListen_, hIOCP_, ULONG_PTR(this), 0))
    {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("WSASocket failed! hr = %08X\n"), hr);
        return hr;
    }


    nRet = bind(socketListen_, addrlocal->ai_addr, (int)addrlocal->ai_addrlen);
    if (nRet == SOCKET_ERROR) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("bind failed! hr = %08X\n"), hr);
        return hr;
    }

    nRet = listen(socketListen_, 5);
    if (nRet == SOCKET_ERROR) {
        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("listen failed! hr = %08X\n"), hr);
        return hr;
    }

    // listen succeed, notify event
    GetApp()->NotifyEvent(EVENT_ID_LISTEN_SUCCESS, MAKEWPARAM(wPort_,0),0);

    nZero = 0;
    nRet = setsockopt(socketListen_, SOL_SOCKET, SO_SNDBUF, (char*)&nZero, sizeof(nZero));
    if (nRet == SOCKET_ERROR) {

        hr = HRESULT_FROM_WIN32(WSAGetLastError());
        LOGF(_T("setsockopt failed! hr = %08X\n"), hr);
        return hr;
    }

    FreeAddrInfo(addrlocal);

    return hr;
}


HRESULT IOCP::GetExtensionFunctions()
{
    HRESULT hr = S_OK;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    GUID guidGetAcceptSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

    DWORD dwBytes = 0;
    if (SOCKET_ERROR == WSAIoctl(
        socketListen_,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &fnAcceptEx_,
        sizeof(fnAcceptEx_),
        &dwBytes,
        NULL,
        NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    if (SOCKET_ERROR == WSAIoctl(
        socketListen_,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidGetAcceptSockAddrs,
        sizeof(guidGetAcceptSockAddrs),
        &fnGetAcceptExSockAddrs_,
        sizeof(fnGetAcceptExSockAddrs_),
        &dwBytes,
        NULL,
        NULL))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }
    return hr;
}


HRESULT IOCP::PostAcceptEx(IOCP_CONNECTION *pConn)
{
    HRESULT hr = S_OK;
    DWORD dwBytes = 0;

    LOGI(_T("AcceptEx wsaol %p\n"), &pConn->wsaol);
    LOGI(_T("AcceptEx Socket %d\n"), pConn->Socket);
    if (!fnAcceptEx_(socketListen_,
        pConn->Socket,
        pConn->wsabuf.buf,
        pConn->wsabuf.len,
        sizeof(sockaddr_in) + 16,
        sizeof(sockaddr_in) + 16,
        &dwBytes,
        &pConn->wsaol))
    {
        DWORD dwErr = WSAGetLastError();
        if (WSA_IO_PENDING != dwErr)
        {
            hr = E_FAIL;
        }
    }

    return hr;
}
HRESULT IOCP::PostRecv(IOCP_CONNECTION *pConn)
{
    int r;
    HRESULT hr = S_OK;
    //DWORD dwBytes = 0;
    DWORD dwFlags = 0;

    //LOGI(_T("WSARecv wsaol %p\n"), pConn->wsaol);
    r = WSARecv(
        pConn->Socket,
        &pConn->wsabuf,
        1,
        NULL,
        &dwFlags,
        &pConn->wsaol,
        NULL);
    if (r == SOCKET_ERROR)
    {
        DWORD dwErr = WSAGetLastError();
        if (WSA_IO_PENDING != dwErr)
        {
            hr = E_FAIL;
            // TODO: shutdown client

        }
    }

    return hr;
}
HRESULT IOCP::PostSend(IOCP_CONNECTION *pConn)
{
    HRESULT hr = S_OK;
    return hr;
}


DWORD WINAPI IOCP::WorkerThreadProc(LPVOID lpParam)
{
    IOCP* pThis = (IOCP*)lpParam;
    return pThis->WorkerThread();
}

DWORD IOCP::WorkerThread()
{
    HRESULT hr;
    LPWSAOVERLAPPED lpOverlapped = NULL;
    DWORD dwIoSize = 0;
    //int nRet;

    PULONG_PTR lpIocpCompletionKey = NULL;
    IOCP_CONNECTION* pConn = NULL;

    
    if (hIOCP_ == 0)
    {
        return 0;
    }

    hr = CreateConnection(&pConn);
    if (FAILED(hr))
    {
        LOGF(_T("CreateConnection failed! hr = %08X\n"), hr);
        return hr;
    }

    hr = PostAcceptEx(pConn);
    if (FAILED(hr))
    {
        LOGF(_T("PostAcceptEx failed! hr = %08X\n"), hr);
        return hr;
    }


    while (1)
    {

        BOOL bSuccess = GetQueuedCompletionStatus(hIOCP_, 
            &dwIoSize,
            (PULONG_PTR)&lpIocpCompletionKey,
            (LPOVERLAPPED*)&lpOverlapped,
            INFINITE);


        pConn = CONTAINING_RECORD(lpOverlapped, IOCP_CONNECTION, wsaol);

        if (lpIocpCompletionKey == PULONG_PTR(this))
        {
            LOGI(_T("listen socket completed\n"));
            LOGI(_T("wsaol %p conn %p\n"), lpOverlapped, pConn);

            ULONG ucnt = InterlockedIncrement(&ulUserAccount_);
            GetApp()->NotifyEvent(EVENT_ID_UPDATE_USERCOUNT, WPARAM(ucnt), 0);

            // create a new connection for accept
            {
                IOCP_CONNECTION* pConnNew;
                hr = CreateConnection(&pConnNew);
                if (FAILED(hr))
                {
                    LOGF(_T("CreateConnection failed! hr = %08X\n"), hr);
                    return hr;
                }

                hr = PostAcceptEx(pConnNew);
                if (FAILED(hr))
                {
                    LOGF(_T("PostAcceptEx failed! hr = %08X\n"), hr);
                    return hr;
                }
            }

            // bind current connection to IOCP
            if ((CreateIoCompletionPort((HANDLE)pConn->Socket, hIOCP_, ULONG_PTR(pConn), 0)) == NULL)
            {
                LOGF(_T("CreateIoCompletionPort failed! error - %d\n"), GetLastError());
                break;
            }


            if (dwIoSize)
            {
                // there are data to be read
                OnRecv(pConn);
                hr = PostRecv(pConn);
                if (FAILED(hr))
                {
                    LOGF(_T("PostRecv failed! hr = %08X\n"), hr);
                    ULONG ucnt = InterlockedDecrement(&ulUserAccount_);
                    GetApp()->NotifyEvent(EVENT_ID_UPDATE_USERCOUNT, WPARAM(ucnt), 0); 
                    DestoryConnection(pConn);
                    break;
                }
                continue;
            }

            if (!bSuccess || (bSuccess && (0 == dwIoSize))) 
            {
                DestoryConnection(pConn);
                ULONG ucnt = InterlockedDecrement(&ulUserAccount_);
                GetApp()->NotifyEvent(EVENT_ID_UPDATE_USERCOUNT, WPARAM(ucnt), 0);
                continue;
            }



        }
        else
        {
            pConn = (IOCP_CONNECTION*)lpIocpCompletionKey;
        }

        if (!bSuccess)
        {
            DWORD dwErr = GetLastError();
            LOGE(_T("GetQueuedCompletionStatus failed - %d\n"), dwErr);
            if (ERROR_ABANDONED_WAIT_0 == dwErr)
            {
                break;
            }
            else if (ERROR_NETNAME_DELETED == dwErr)
            {
                DestoryConnection(pConn);
                ULONG ucnt = InterlockedDecrement(&ulUserAccount_);
                GetApp()->NotifyEvent(EVENT_ID_UPDATE_USERCOUNT, WPARAM(ucnt), 0);
                continue;
            }
        }


        OnRecv(pConn);
        // post Recv
        hr = PostRecv(pConn);
        if (FAILED(hr))
        {
            LOGF(_T("PostRecv failed! hr = %08X\n"), hr);
            ULONG ucnt = InterlockedDecrement(&ulUserAccount_);
            GetApp()->NotifyEvent(EVENT_ID_UPDATE_USERCOUNT, WPARAM(ucnt), 0); 
            DestoryConnection(pConn);
            break;
        }
    }

    return 0;
}

HRESULT IOCP::OnRecv(IOCP_CONNECTION* pConn)
{
    LOGI(_T("OnRecv %d\n"), *(DWORD*)pConn->buffer);
    LOGI(_T("Tag %x Tag2 %x\n"), pConn->tag,pConn->tag2);


    return S_OK;
}

HRESULT IOCP::OnSend(IOCP_CONNECTION* pConn)
{
    LOGI(_T("OnSend\n"));
    return S_OK;
}


HRESULT IOCP::CreateConnection(IOCP_CONNECTION* *ppConn)
{
    HRESULT hr = E_OUTOFMEMORY;
    CCSLock lock(&csListConn_);
    IOCP_CONNECTION* conn = (IOCP_CONNECTION*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IOCP_CONNECTION));
    if (conn)
    {        
        conn->Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        conn->wsabuf.buf = conn->buffer;
        conn->wsabuf.len = MAX_BUFF_SIZE-100; // TODO: Workaround to avoid heap corruption
        //InsertTailList(&listConnection_, &conn->ListEntry);
        conn->tag = 0;
        conn->tag2 = 0xabcd;

        *ppConn = conn;
        hr = S_OK;

        // Add user account

        
    }

    LOGI(_T("new connect %p\n"), conn);

    return hr;
}

HRESULT IOCP::DestoryConnection(IOCP_CONNECTION* pConn)
{
    CCSLock lock(&csListConn_);

    while (!HasOverlappedIoCompleted((LPOVERLAPPED)&pConn->wsaol)) Sleep(0);

    closesocket(pConn->Socket);

    LOGI(_T("delete connect %p\n"), pConn);

    //RemoveEntryList(&pConn->ListEntry);

    HeapFree(GetProcessHeap(), 0, pConn);
    return S_OK;
}