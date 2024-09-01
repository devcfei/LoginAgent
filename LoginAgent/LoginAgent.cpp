#include "framework.h"



LoginAgent::LoginAgent()
{
    InitializeDefaultConfig();
}

LoginAgent::~LoginAgent()
{
}

HRESULT LoginAgent::Initialize(LPCTSTR lpszCfgFile)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

    if (!lpszCfgFile)
    {
        LOGE(_T("%s failed"), __FUNCTION__);
        hr = E_INVALIDARG;
        return hr;
    }

    BOOL bFileExist = FALSE;
    DWORD dwAttrib = GetFileAttributes(lpszCfgFile);
    if (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        bFileExist = TRUE;
        hr = S_OK;
    }

    if (!bFileExist)
    {
        LOGW(_T("LoginAgent.ini doesn't exist, create it"));
        hr = S_OK;
        WriteConfig(lpszCfgFile);
    }
    else
    {
        ReadConfig(lpszCfgFile);
    }

    hr = IOCP::Initialize(wPort_);

    return hr;
}


HRESULT LoginAgent::Start()
{
    HRESULT hr = S_OK;

    hr = IOCP::Start();

    return hr;
}

HRESULT LoginAgent::Stop()
{
    HRESULT hr = S_OK;
    
    hr = IOCP::Stop();

    return hr;
}


HRESULT LoginAgent::InitializeDefaultConfig()
{
    HRESULT hr = S_OK;

    nVerHigh_ = 700;
    nVerLow_ = 700;

    nMode_ = 0;
    wPort_ = 3550;
    wZAPort_ = 3200;
    nStartID_ = 345856;
    nHashType_ = 0;
    bExpiration_ = TRUE;

    StringCchCopy(szMsgBanned_, 256, _T("Your IP has been banned from this server"));
    StringCchCopy(szMsgMaintainace_, 256, _T("Server is down for maintainance"));
    StringCchCopy(szMsgWrongAccount_, 256, _T("Your login information was incorrect"));
    StringCchCopy(szMsgDuplicateUser_, 256, _T("Your account is already connected to another user"));
    StringCchCopy(szMsgManyFailed_, 256, _T("Your ip has too many failed attempts, Please try again in 10 minutes"));
    StringCchCopy(szMsgAccountExpired_, 256, _T("Your account is expired. Please renew your membership"));
    StringCchCopy(szMsgAccountExpiredInfo_, 256, _T("Your account expires after {0} days {1} hours {2} minutes"));


    StringCchCopy(szIP_, 16, _T("127.0.0.1"));
    wDBPort_ = 1433;
    StringCchCopy(szDBID_, 64, _T("sa"));
    StringCchCopy(szDBPassword_, 64, _T("123456"));
    StringCchCopy(szDBCatalog_, 64, _T("ASD"));

    nServerGroup_ = 1;
    ServerInfo si = { 0 };
    StringCchCopy(si.name, 256, _T("A3 Server Name"));

    vecServerGroup_.push_back(si);

    return hr;
}

HRESULT LoginAgent::ReadConfig(LPCTSTR lpszCfgFile)
{
    HRESULT hr = S_OK;

    // [VERSIONINFO]
    nVerHigh_ = GetPrivateProfileInt(_T("VERSIONINFO"), _T("HIGH"), 700, lpszCfgFile);
    nVerLow_ = GetPrivateProfileInt(_T("VERSIONINFO"), _T("LOW"), 700, lpszCfgFile);

    // [STARTUP]
    nMode_ = GetPrivateProfileInt(_T("STARTUP"), _T("MODE"), 0, lpszCfgFile);
    wPort_ = GetPrivateProfileInt(_T("STARTUP"), _T("PORT"), 3550, lpszCfgFile);
    wZAPort_ = GetPrivateProfileInt(_T("STARTUP"), _T("ZONEAGENTLISTENPORT"), 3200, lpszCfgFile);
    nStartID_ = GetPrivateProfileInt(_T("STARTUP"), _T("STARTID"), 345856, lpszCfgFile);
    nHashType_ = GetPrivateProfileInt(_T("STARTUP"), _T("HASHTYPE"), 0, lpszCfgFile);
    bExpiration_ = GetPrivateProfileInt(_T("STARTUP"), _T("EXPIRATION"), 0, lpszCfgFile);

    GetPrivateProfileString(_T("STARTUP"), _T("MSG_BANNED"), _T("Your IP has been banned from this server"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_MAINTAINANCE"), _T("Server is down for maintainance"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_WRONGACCOUNT"), _T("Your login information was incorrect"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_DUPLICATEUSER"), _T("Your account is already connected to another user"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_MANYFAILED"), _T("Your ip has too many failed attempts, Please try again in 10 minutes"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_EXPIRED"), _T("Your account is expired. Please renew your membership"), szMsgBanned_, 256, lpszCfgFile);
    GetPrivateProfileString(_T("STARTUP"), _T("MSG_EXPIREINFO"), _T("Your account expires after {0} days {1} hours {2} minutes"), szMsgBanned_, 256, lpszCfgFile);

    // [LOGIN_DB]
    GetPrivateProfileString(_T("LOGIN_DB"), _T("IP"), _T("127.0.0.1"), szIP_, 16, lpszCfgFile);
    wDBPort_ = GetPrivateProfileInt(_T("LOGIN_DB"), _T("PORT"), 1433, lpszCfgFile);
    GetPrivateProfileString(_T("LOGIN_DB"), _T("ID"), _T("sa"), szDBID_, 64, lpszCfgFile);
    GetPrivateProfileString(_T("LOGIN_DB"), _T("PWD"), _T("123456"), szDBPassword_, 64, lpszCfgFile);
    GetPrivateProfileString(_T("LOGIN_DB"), _T("CATALOG"), _T("ASD"), szDBCatalog_, 64, lpszCfgFile);

    // [SERVER_GROUP]
    nServerGroup_ = GetPrivateProfileInt(_T("SERVER_GROUP"), _T("COUNT"), 1, lpszCfgFile);
    for (UINT i = 0; i < nServerGroup_; ++i)
    {
        TCHAR key[32];
        ServerInfo si = { 0 };

        StringCchPrintf(key, 32, _T("ID%d"), i);
        si.id = GetPrivateProfileInt(_T("SERVER_GROUP"), key, i, lpszCfgFile);

        StringCchPrintf(key, 32, _T("NAME%d"), i);
        GetPrivateProfileString(_T("SERVER_GROUP"), key, _T("A3 Server Name"), si.name, 256, lpszCfgFile);

        vecServerGroup_.push_back(si);

    }

    return hr;
}

HRESULT LoginAgent::WriteConfig(LPCTSTR lpszCfgFile)
{
    HRESULT hr = S_OK;

    TCHAR value[32];
    // [VERSIONINFO]
    StringCchPrintf(value, 32, _T("%d"), nVerHigh_);
    WritePrivateProfileString(_T("VERSIONINFO"), _T("HIGH"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), nVerLow_);
    WritePrivateProfileString(_T("VERSIONINFO"), _T("LOW"), value, lpszCfgFile);

    // [STARTUP]
    StringCchPrintf(value, 32, _T("%d"), nMode_);
    WritePrivateProfileString(_T("STARTUP"), _T("MODE"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), wPort_);
    WritePrivateProfileString(_T("STARTUP"), _T("PORT"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), wZAPort_);
    WritePrivateProfileString(_T("STARTUP"), _T("ZONEAGENTLISTENPORT"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), nStartID_);
    WritePrivateProfileString(_T("STARTUP"), _T("STARTID"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), nHashType_);
    WritePrivateProfileString(_T("STARTUP"), _T("HASHTYPE"), value, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), bExpiration_);
    WritePrivateProfileString(_T("STARTUP"), _T("EXPIRATION"), value, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_BANNED"), szMsgBanned_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_MAINTAINANCE"), szMsgMaintainace_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_WRONGACCOUNT"), szMsgWrongAccount_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_DUPLICATEUSER"), szMsgDuplicateUser_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_MANYFAILED"), szMsgManyFailed_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_EXPIRED"), szMsgAccountExpired_, lpszCfgFile);
    WritePrivateProfileString(_T("STARTUP"), _T("MSG_EXPIREINFO"), szMsgAccountExpiredInfo_, lpszCfgFile);

    // [LOGIN_DB]
    WritePrivateProfileString(_T("LOGIN_DB"), _T("IP"), szIP_, lpszCfgFile);
    StringCchPrintf(value, 32, _T("%d"), wDBPort_);
    WritePrivateProfileString(_T("LOGIN_DB"), _T("PORT"), value, lpszCfgFile);
    WritePrivateProfileString(_T("LOGIN_DB"), _T("ID"), szDBID_, lpszCfgFile);
    WritePrivateProfileString(_T("LOGIN_DB"), _T("PWD"), szDBPassword_, lpszCfgFile);
    WritePrivateProfileString(_T("LOGIN_DB"), _T("CATALOG"), szDBCatalog_, lpszCfgFile);


    // [SERVER_GROUP]
    StringCchPrintf(value, 32, _T("%d"), nServerGroup_);
    WritePrivateProfileString(_T("SERVER_GROUP"), _T("COUNT"), value, lpszCfgFile);
    for (UINT i = 0; i < nServerGroup_; ++i)
    {
        TCHAR key[32];
        StringCchPrintf(key, 32, _T("ID%d"), i);
        StringCchPrintf(value, 32, _T("%d"), vecServerGroup_[i].id);
        WritePrivateProfileString(_T("SERVER_GROUP"), key, value, lpszCfgFile);

        StringCchPrintf(key, 32, _T("NAME%d"), i);
        WritePrivateProfileString(_T("SERVER_GROUP"), key, vecServerGroup_[i].name, lpszCfgFile);
    }

    return hr;
}
