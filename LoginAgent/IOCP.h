#pragma once

#if !defined(_WINSOCKAPI_)
#error "IOCP need winsock2.h"
#endif

#define MAX_BUFF_SIZE       4096*2
#define MAX_WORKER_THREAD   16

typedef enum _IOCP_OPERATION {
    IOCP_OPERATION_ACCEPT,
    IOCP_OPERATION_RECV,
    IOCP_OPERATION_SEND
} IOCP_OPERATION, * PIOCP_OPERATION;



typedef struct _IOCP_CONNECTION
{
    WSAOVERLAPPED wsaol;
    //IOCP_OPERATION IocpOpteration;
    SOCKET Socket;
    WSABUF wsabuf;
    DWORD tag;
    CHAR buffer[MAX_BUFF_SIZE];
    DWORD tag2;
    //PPER_IO_CONTEXT pPerIoCtx;

    LIST_ENTRY ListEntry;

}IOCP_CONNECTION;


class IOCP
{
public:
    IOCP();
    ~IOCP();
    HRESULT Initialize(WORD wPort);
    HRESULT Start();
    HRESULT Stop();

    virtual HRESULT OnRecv(IOCP_CONNECTION* pConn);
    virtual HRESULT OnSend(IOCP_CONNECTION* pConn);

    HRESULT PostRecv(IOCP_CONNECTION* pConn);
    HRESULT PostSend(IOCP_CONNECTION* pConn);

private:
    WORD wPort_;

    DWORD dwThreadCount_;

    HANDLE hIOCP_;

    HANDLE* hThreadArr_;
    static DWORD WINAPI WorkerThreadProc(LPVOID lpParam);
    DWORD WorkerThread();

    HRESULT StartListen();
    SOCKET socketListen_;
    ULONG_PTR socketListenCompKey_;

    // AcceptEx and GetAcceptExSockAddrs
    HRESULT GetExtensionFunctions();
    LPFN_ACCEPTEX fnAcceptEx_;
    LPFN_GETACCEPTEXSOCKADDRS fnGetAcceptExSockAddrs_;


private:
    HRESULT PostAcceptEx(IOCP_CONNECTION *pConn);


private:
    // connection
    CRITICAL_SECTION csListConn_;
    LIST_ENTRY listConnection_;

    HRESULT CreateConnection(IOCP_CONNECTION** ppConn);
    HRESULT DestoryConnection(IOCP_CONNECTION* pConn);

    ULONG ulUserAccount_ = 0;
};