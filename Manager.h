
#ifndef _APP_MANAGER_HEADER_
#define _APP_MANAGER_HEADER_
#include <map>
#include <string>

#include "app.h"
#include "DesktopMgr.h"
#include "Config.h"
#include "JSManager.h"
#include "AppManager.h"

#define DESTROY_PTR(ptr) { if(ptr) delete ptr; ptr = nullptr; }


class GLOBAL
{
public:
    static GLOBAL& get()
    {
        static GLOBAL instance;

        return instance;
    }

    CefRefPtr<MainApp>                   cefapp_                 = nullptr;
    CefRefPtr<CefBrowser>                browser_                = nullptr;
    CefRefPtr<JSManager>                 jshandler_              = nullptr;
    CefRefPtr<CefV8Context>              context_                = nullptr;

    HWND                                 window_                  = nullptr;
    HRGN                                 draggable_region_        = ::CreateRectRgn(0, 0, 0, 0);
    HINSTANCE                            hInstance_               = nullptr;
    NOTIFYICONDATA*                      nidApp_                  = nullptr;

    Conf*                                config_                  = nullptr;
    APP_MGR*                             appmgr_                  = nullptr;
    


private:
    GLOBAL() {};
    ~GLOBAL() {};

    GLOBAL(const GLOBAL&) = delete;

    std::map<std::string, int> integer_container_;
    std::map<std::string, std::string> string_container_;


public:
    bool Init(HINSTANCE hInstance);
    void InitShellIcon();
    void InitConfig();

    void Destroy();

    // functions
    static HWND                     GetWindow()     { return GLOBAL::get().window_;  }
    static CefRefPtr<CefBrowser>    GetBrowser()    { return GLOBAL::get().browser_; }
    static CefRefPtr<MainApp>       GetCefApp()     { return GLOBAL::get().cefapp_;  }

    static Conf*                    GetConfig()     { return GLOBAL::get().config_; }
    static APP_MGR*                 GetAppMgr()     { return GLOBAL::get().appmgr_; }

    // render process
    static CefRefPtr<JSManager>     GetJSHandler() { return GLOBAL::get().jshandler_; }
    static CefRefPtr<CefV8Context>  GetGlobalContext() { return GLOBAL::get().context_; }
};


inline void SetJSValue(const char* var_name, int value)
{
    if (!CefCurrentlyOn(TID_RENDERER) && GLOBAL::GetBrowser())
    {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JS_VAL_SYNC_INT");
        CefRefPtr<CefListValue> argss = msg->GetArgumentList();
        argss->SetString(0, var_name);
        argss->SetInt(1, value);
        GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);
    }
}

inline void SetJSValue(const char* var_name, const char* value)
{
    if (!CefCurrentlyOn(TID_RENDERER) && GLOBAL::GetBrowser())
    {
        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JS_VAL_SYNC_STR");
        CefRefPtr<CefListValue> argss = msg->GetArgumentList();
        argss->SetString(0, var_name);
        argss->SetString(1, value);
        GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);
    }
}


inline bool SetRegValue(const char* location, const char* key, const char* value)
{
    HKEY hkey = nullptr;
    RegOpenKeyA(HKEY_CURRENT_USER, location, &hkey); //Creates a key     

    auto res = RegSetValueExA(hkey, key, 0, REG_SZ, (BYTE*)value, strlen(value)+1);

    if (res != ERROR_SUCCESS)
    {
        printf("REGISTRY VALUE FAILED, key %s, value %s \n", key, value);
        RegCloseKey(hkey);
        return false;
    }

    RegCloseKey(hkey);

    return false;
}

inline bool RemoveReg(const char* location, const char* key)
{
    HKEY hkey = nullptr;
    RegOpenKeyA(HKEY_CURRENT_USER, location, &hkey); //Creates a key     

    auto res = RegDeleteValueA(hkey, key);

    if (res != ERROR_SUCCESS)
    {
        printf("FAIL TO REMOVE REGISTRY key: %s, error %d \n", key, res);
        RegCloseKey(hkey);
        return false;
    }

    RegCloseKey(hkey);


    return true;
}

inline bool GetReg(const char* location, const char* key, std::string &result)
{
    HKEY hkey = nullptr;
    RegOpenKeyA(HKEY_CURRENT_USER, location, &hkey); //Creates a key     

    char szBuff[500];
    DWORD size = sizeof(szBuff);

    auto res = RegQueryValueExA(hkey, key, 0, NULL, (LPBYTE)szBuff, &size);

    if (res != ERROR_SUCCESS)
    {
        result = "";
        printf("FAIL TO GET REGISTRY key: %s, error %d \n", key, res);
        RegCloseKey(hkey);
        return false;
    }

    result = szBuff;

    RegCloseKey(hkey);

    return true;
}

namespace signal
{
    enum
    {
        SIGNAL_SHUTDOWN,
        SIGNAL_MINIMIZE,
        SIGNAL_OFF_DESKMGR,
        SIGNAL_ON_DESKMGR,
        SIGNAL_INIT_DESKMGR,
        SIG_TEST,
    };


    void SendSignalToRender(int signalId);
    void SendSignalToMain(int signalId);
    bool ProcessSignal(int signal);
};


#endif
