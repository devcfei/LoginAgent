#include "framework.h"


HRESULT App::Initialize(HINSTANCE hInstance)
{
    HRESULT hr = S_OK;
    
    // save hInstance;
    hInst_ = hInstance;

    // Get application path
    GetModuleFileName(NULL, szAppPath_, MAX_PATH);
    PathRemoveFileSpec(szAppPath_);

    // build configuration file path and initialize LoginAgent
    TCHAR szCfgFile[MAX_PATH];
    StringCchCopy(szCfgFile, MAX_PATH, szAppPath_);
    StringCchCat(szCfgFile, MAX_PATH, _T("\\LoginAgent.ini"));

    // initialize LoginAgent
    hr = LA_.Initialize(szCfgFile);

    return hr;
}

HRESULT App::Run()
{
    HRESULT hr = S_OK;
    DialogBoxParam(hInst_, MAKEINTRESOURCE(IDD_MAIN), NULL, DialogProc, (LPARAM)this);
    return hr;
}

HRESULT App::Start()
{
    HRESULT hr = S_OK;
    hr = LA_.Start();
    return hr;
}

HRESULT App::Stop()
{
    HRESULT hr = S_OK;
    hr = LA_.Stop();
    return hr;
}


INT_PTR CALLBACK App::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR r = (INT_PTR)FALSE;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:

        r = GetApp()->OnInitDialog(hDlg, message, wParam, lParam);
        break;

    case WM_CTLCOLORSTATIC:
        r = GetApp()->OnColorStatic(hDlg, message, wParam, lParam);
        break;

    case WM_COMMAND:
        r = GetApp()->OnCommand(hDlg, message, wParam, lParam);
        break;
    }

    return r;
}


INT_PTR App::OnInitDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Save HWND
    hDlg_ = hDlg;

    // make window in the center
    RECT rect;
    GetWindowRect(hDlg, &rect);
    int nWidth = rect.right - rect.left;
    int nHeight = rect.bottom - rect.top;

    int iScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    int iScreenHeight = GetSystemMetrics(SM_CYSCREEN);

    int X = (iScreenWidth - nWidth) / 2;
    int Y = (iScreenHeight - nHeight) / 2;
    SetWindowPos(hDlg, NULL, X, Y, nWidth, nHeight, SWP_NOSIZE);

    // set dialog icon
    HICON hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(IDI_FAVICON));
    SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

    // bold the checkbox title
    HWND hwnd;
    HFONT hFontB, hFont;
    LOGFONT lf;
    hwnd = GetDlgItem(hDlg, IDC_CHECK_SERVER_MAINTAINANCE);
    hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    GetObject(hFont, sizeof(LOGFONT), &lf);
    lf.lfWeight = FW_BOLD;
    hFontB = CreateFontIndirect(&lf);
    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFontB, TRUE);


    // create a global brush for log
    hbrBkgnd_ = CreateSolidBrush(RGB(0, 0, 0));


    //

    // Initialize App
    HRESULT hr;
    hr = GetApp()->Start();
    if (FAILED(hr))
    {
        GetApp()->Stop();
        LOGF(_T("App startup failed!\n"));
    }
        
    PrintMessage(_T("App initialize success!\n"));



    return (INT_PTR)TRUE;
}


INT_PTR App::OnColorStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR r = (INT_PTR)FALSE;
    HDC hdc = (HDC)wParam;
    HWND hwnd = (HWND)lParam;

    int iCtrlID = GetDlgCtrlID(hwnd);

    switch (iCtrlID)
    {
    case IDC_EDIT_LOG:
        SetTextColor(hdc, RGB(200, 200, 200));
        SetBkColor(hdc, RGB(0, 0, 0));
        r = (INT_PTR)hbrBkgnd_;
        break;

    }

    return r;
}


INT_PTR App::OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR r = (INT_PTR)FALSE;

    WORD idCtrlID = LOWORD(wParam);

    switch (idCtrlID)
    {
    case IDCANCEL:
        GetApp()->Stop();        

        EndDialog(hDlg, LOWORD(wParam));
        r = (INT_PTR)TRUE;
        break;

    case IDC_BTN_RELOAD:
        MessageBox(hDlg, _T("Reload settings"), _T("Reload"), MB_OKCANCEL);
        r = (INT_PTR)TRUE;
        break;
    }
    return r;
}


HRESULT App::PrintMessage(LPCTSTR fmt, ...)
{
    HRESULT hr;
    TCHAR szMessageBuffer[1024] = { 0 };
    va_list valist;
    va_start(valist, fmt);
    hr = StringCchVPrintf(szMessageBuffer, 1024, fmt, valist);
    if (FAILED(hr))
    {
        return hr;
    }
    va_end(valist);

    logMsg_ += szMessageBuffer;
    logMsg_ += _T("\r\n");

    HWND hWndEditLog = GetDlgItem(hDlg_, IDC_EDIT_LOG);
    SetWindowText(hWndEditLog, logMsg_.c_str());

    return hr;
}


HRESULT App::NotifyEvent(EVENT_ID eid, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    switch (eid)
    {
    case EVENT_ID_LISTEN_SUCCESS:
    {
        WORD wPort = LOWORD(wParam);
        HWND hWnd = GetDlgItem(hDlg_, IDC_STATIC_PORT);
        TCHAR szText[16];
        StringCchPrintf(szText, 16, _T("%d"),wPort);
        SetWindowText(hWnd, szText);

        break;
    }
    case EVENT_ID_UPDATE_USERCOUNT:
    {
        HWND hWnd = GetDlgItem(hDlg_, IDC_STATIC_USERCOUNT);
        TCHAR szText[16];
        StringCchPrintf(szText, 16, _T("%d"), wParam);
        SetWindowText(hWnd, szText);
    
        break;
    }
    default:
        break;
    }


    return hr;
}

