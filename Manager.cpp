#include "handler.h"
#include "app.h"
#include "Config.h"
#include "JSManager.h"
#include "AppManager.h"
#include "Manager.h"
#include <mutex>
#include "Win32-CefIntegration.h"
#include "AppManager.h"
#include "Resource.h"



void GLOBAL::Destroy()
{

    DESTROY_PTR(appmgr_);
    DESTROY_PTR(config_);
    
    if (nidApp_)
    {
        Shell_NotifyIcon(NIM_DELETE, nidApp_);
        DESTROY_PTR(nidApp_);
    }

}

bool GLOBAL::Init(HINSTANCE hInstance)
{
    hInstance_ = hInstance;
    cefapp_ = new MainApp(); // cef app

    printf("Init global pointers. (hInstance, cefapp) \n");

    return true;
}

void GLOBAL::InitConfig()
{
    config_ = new Conf; // initialize global config (json)
    printf("Init config file. \n");
}


// icon data
void GLOBAL::InitShellIcon()
{

    printf("Init Shell Icon. \n");
    nidApp_ = new NOTIFYICONDATA;

    HICON hMainIcon             = LoadIcon(hInstance_, (LPCTSTR)MAKEINTRESOURCE(IDI_QMANAGER));

    nidApp_->cbSize             = sizeof(NOTIFYICONDATA);
    nidApp_->hWnd               = window_;
    nidApp_->uID                = IDI_WIN32CEFINTEGRATION;
    nidApp_->uFlags             = NIF_ICON | NIF_INFO | NIF_SHOWTIP | NIF_MESSAGE;
    nidApp_->hIcon              = hMainIcon;
    nidApp_->uVersion =         NOTIFYICON_VERSION_4;

   // _snwprintf_s(nidApp_->szTip, ARRAYSIZE(nidApp_->szTip), L"Test application");

    nidApp_->uCallbackMessage   = WM_USER_SHELLICON;

    Shell_NotifyIcon(NIM_ADD, nidApp_);
}



// signals
namespace signal
{

    bool ProcessSignal(int signal)
    {
        printf("SIGNAL RECEIVED, ID %d\n", signal);
        switch (signal)
        {
        case SIGNAL_SHUTDOWN:
        {
            ShutDown();
        }
        return true;

        case SIGNAL_MINIMIZE:
        {
            if (::IsWindowVisible(GLOBAL::GetWindow()))
            {
                ShowWindow(GLOBAL::GetWindow(), SW_HIDE);
                GLOBAL::GetConfig()->SaveData();
            }

        }
        return true;

        case SIGNAL_ON_DESKMGR:
        {
            auto deskMgr = GLOBAL::GetAppMgr()->desktopMgr;
            if (deskMgr && !GLOBAL::GetConfig()->GetInt("desk_mgr"))
            {
                deskMgr->Run();
                GLOBAL::GetConfig()->SetValue("desk_mgr", 1);
            }
                
        }
        return true;

        case SIGNAL_OFF_DESKMGR:
        {
            auto deskMgr = GLOBAL::GetAppMgr()->desktopMgr;
            if (deskMgr && GLOBAL::GetConfig()->GetInt("desk_mgr"))
            {
                deskMgr->QuitLopp();
                GLOBAL::GetConfig()->SetValue("desk_mgr", 0);
            }
               
        }
        return true;

        // test mode
        case signal::SIG_TEST:
        {

        }
        return true;

        default:
            return false;
        }
 
        return false;
    }

    void SendSignalToMain(int signalId)
    {
        if (!GLOBAL::GetBrowser())
            return;

        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSCommand");
        CefRefPtr<CefListValue> argss = msg->GetArgumentList();
        argss->SetInt(0, signalId);
        GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
    }

    void SendSignalToRender(int signalId)
    {
        if (!GLOBAL::GetBrowser())
            return;

        CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("JSCommand");
        CefRefPtr<CefListValue> argss = msg->GetArgumentList();

        argss->SetInt(0, signalId);
        GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, msg);
    }
}


