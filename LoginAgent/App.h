#pragma once

class LoginAgentSession : public Session
{
public:
    LoginAgentSession()
        :Session()
    {
        bufSendLen_ = 4096;
        bufSend_ = new BYTE[bufSendLen_];
    }
    virtual ~LoginAgentSession()
    {
        delete[] bufSend_;
    }

    virtual HRESULT ProcessEvent(void* pbuff, size_t len);
    virtual HRESULT OnClose();
private:
    BYTE* bufSend_;
    size_t bufSendLen_;
};


class App: public IEvent, public LibEvent
{
public:
    HRESULT Initialize(HINSTANCE hInstance);
    HRESULT Run();
    HRESULT Start();
    HRESULT Stop();
    HRESULT PrintMessage(LPCTSTR fmt, ...);

    LoginAgent* GetLA()
    {
        return &LA_;
    }

    // IEvent
    HRESULT NotifyEvent(EVENT_ID eid, WPARAM wParam, LPARAM lParam);


private:
    DWORD dwUserCount_ = 0;
    virtual HRESULT OnConnect(Session** ppSession)
    {
        HRESULT hr = S_OK;

        Session* pSession = new LoginAgentSession;
        if (!pSession)
        {
            hr = E_OUTOFMEMORY;
            return hr;
        }

        *ppSession = pSession;

        NotifyEvent(EVENT_ID_USER_CONNECTED, 0, 0);

        return hr;
    }

    virtual HRESULT OnDisconnect(Session* pSession)
    {
        HRESULT hr = S_OK;

        delete pSession;

        return hr;
    }
private:
    TCHAR szAppPath_[MAX_PATH];
    LoginAgent LA_;

    HINSTANCE hInst_;
    HWND    hDlg_;
    HBRUSH hbrBkgnd_;
   
    // Log
    CRITICAL_SECTION csLog_;
    std::tstring logMsg_;
    DWORD dwLineCount_ = 0;;

    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnInitDialog(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnColorStatic(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnCommand(HWND, UINT, WPARAM, LPARAM);

    

};