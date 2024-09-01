#pragma once



class LoginAgent
{
public:
    LoginAgent();
    ~LoginAgent();

    HRESULT Initialize(LPCTSTR lpszCfgFile);

    HRESULT Start();
    HRESULT Stop();

    HRESULT GetServerName(int nIndex, LPTSTR lpszName, int nSize)
    {
        return StringCchCopy(lpszName, nSize, vecServerGroup_[nIndex].name);
    }

    WORD GetPort()
    {
        return wPort_;
    }
private:
    HRESULT InitializeDefaultConfig();
    HRESULT ReadConfig(LPCTSTR lpszCfgFile);
    HRESULT WriteConfig(LPCTSTR lpszCfgFile);

private:
    // config file
    // VERSIONINFO
    UINT nVerHigh_;
    UINT nVerLow_;
    // STARTUP
    UINT nMode_;
    WORD wPort_;
    WORD wZAPort_;
    UINT nStartID_;
    UINT nHashType_;
    BOOL bExpiration_;
    TCHAR szMsgBanned_[256];
    TCHAR szMsgMaintainace_[256];
    TCHAR szMsgWrongAccount_[256];
    TCHAR szMsgDuplicateUser_[256];
    TCHAR szMsgManyFailed_[256];
    TCHAR szMsgAccountExpired_[256];
    TCHAR szMsgAccountExpiredInfo_[256];
    // LOGIN_DB
    TCHAR szIP_[16];
    WORD wDBPort_;
    TCHAR szDBID_[64];
    TCHAR szDBPassword_[64];
    TCHAR szDBCatalog_[64];

    // SERVER_GROUP
    UINT nServerGroup_;
    struct ServerInfo
    {
        UINT id;
        TCHAR name[256];
    };
    std::vector< ServerInfo> vecServerGroup_;
};