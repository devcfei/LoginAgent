#pragma once



class App: public IEvent
{
public:
    HRESULT Initialize(HINSTANCE hInstance);
    HRESULT Run();
    HRESULT Start();
    HRESULT Stop();
    HRESULT PrintMessage(LPCTSTR fmt, ...);

    // IEvent
    HRESULT NotifyEvent(EVENT_ID eid, WPARAM wParam, LPARAM lParam);

private:
    TCHAR szAppPath_[MAX_PATH];
    LoginAgent LA_;

    HINSTANCE hInst_;
    HWND    hDlg_;
    HBRUSH hbrBkgnd_;
    std::tstring logMsg_;

    static INT_PTR CALLBACK DialogProc(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnInitDialog(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnColorStatic(HWND, UINT, WPARAM, LPARAM);
    INT_PTR OnCommand(HWND, UINT, WPARAM, LPARAM);

    
};