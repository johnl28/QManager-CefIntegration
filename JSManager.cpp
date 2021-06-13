#include <functional>
#include "include/base/cef_lock.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "handler.h"
#include "JSManager.h"
#include "include/cef_app.h"
#include "Manager.h"
#include "include/wrapper/cef_closure_task.h"


#define JS_FUNC(name) void name(CefRefPtr<CefV8Value>& retVal, const CefV8ValueList& args, CefRefPtr<CefBrowser> browser)

func_container JSManager::functions {};

/// <summary>
/// 
/// </summary>
/// <param name=""></param>
JS_FUNC(Quit)
{
    if (args.size())
    {
        signal::SendSignalToMain(signal::SIGNAL_MINIMIZE);   
        // shutdown the render process while it is minimized
        CefShutdown();
        std::exit(EXIT_SUCCESS);
        return;
    }

    signal::SendSignalToMain(signal::SIGNAL_SHUTDOWN);
    
}

JS_FUNC(GetTitle)
{
    retVal = CefV8Value::CreateString("QManager BETA");
}


JS_FUNC(Refresh)
{
    browser->Reload();
}

JS_FUNC(ToggleDesktopMgr)
{
    if (!args.size())
        return;
    //apps::deskMgr->Run();

    GLOBAL::GetConfig()->SetValue("desk_mgr", !args[0]->GetIntValue());

    if (args[0]->GetIntValue())
    {
        signal::SendSignalToMain(signal::SIGNAL_OFF_DESKMGR);
        return;
    }
        
    signal::SendSignalToMain(signal::SIGNAL_ON_DESKMGR);
}


namespace config
{
    JS_FUNC(GetStrValue)
    {
        if (!args.size())
        {
            return;
        }

        auto value = GLOBAL::GetConfig()->GetString(args[0]->GetStringValue().ToString().c_str());
        retVal = CefV8Value::CreateString(value);
    }

    JS_FUNC(GetIntValue)
    {
        if (!args.size())
        {
            return;
        }

        auto value = GLOBAL::GetConfig()->GetInt(args[0]->GetStringValue().ToString().c_str());
        retVal = CefV8Value::CreateInt(value);
    }


    JS_FUNC(SetStrValue)
    {
        if (!args.size())
        {
            return;
        }

        auto index = args[0]->GetStringValue().ToString();
        auto value = args[1]->GetStringValue().ToString();

        if (GLOBAL::GetBrowser())
        {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SYNC_CONF_STR");
            CefRefPtr<CefListValue> argss = msg->GetArgumentList();
            argss->SetString(0, index);
            argss->SetString(1, value);
            GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
        }


        GLOBAL::GetConfig()->SetValue(index.c_str(), value.c_str());
    }

    JS_FUNC(SetIntValue)
    {
        if (!args.size())
        {
            return;
        }


        auto index = args[0]->GetStringValue().ToString();
        auto value = args[1]->GetIntValue();

        if (GLOBAL::GetBrowser())
        {
            CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("SYNC_CONF_INT");
            CefRefPtr<CefListValue> argss = msg->GetArgumentList();
            argss->SetString(0, index);
            argss->SetInt(1, value);
            GLOBAL::GetBrowser()->GetMainFrame()->SendProcessMessage(PID_BROWSER, msg);
        }


        GLOBAL::GetConfig()->SetValue(index.c_str(), value);
    }

    // desktop colors manager
    HKEY hkey = nullptr;

    JS_FUNC(GetWindowsColor)
    {
        if (!args.size())
        {
            return;
        }

        if (!hkey)
        {
            RegCreateKeyA(HKEY_CURRENT_USER, "Control Panel\\Colors", &hkey); //Creates a key     
        }

        auto index = args[0]->GetStringValue().ToString();

        char szBuff[500] = {0};
        DWORD size = sizeof(szBuff);
        RegQueryValueExA(hkey, index.c_str(), 0, NULL, (LPBYTE)szBuff, &size);
        printf(szBuff);

        retVal = CefV8Value::CreateString(szBuff);
    }

    JS_FUNC(SetWindowsColor)
    {
        if (!args.size())
        {
            return;
        }

        if (!hkey)
        {
            RegCreateKeyA(HKEY_CURRENT_USER, "Control Panel\\Colors", &hkey); //Creates a key     
        }

        auto index = args[0]->GetStringValue().ToString();
        auto value = args[1]->GetStringValue().ToString();

        auto res = RegSetValueExA(hkey, index.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), strlen(value.c_str()) + 1);

    }


    // Startup
    JS_FUNC(ToggleStartup)
    {

        auto hwnd = FindWindowA("Shell_TrayWnd", nullptr);
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, RGB(255, 0, 0), 200, LWA_ALPHA);
    }


    // taskbar
    JS_FUNC(ChangeTaskTransparency)
    {
        if (!args.size())
        {
            return;
        }
        auto value = args[0]->GetIntValue();

        auto hwnd = FindWindowA("Shell_TrayWnd", nullptr);
        auto res = SetWindowLong(hwnd, GWL_HINSTANCE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

        SetLayeredWindowAttributes(hwnd, RGB(0xff, 0xff, 0xfe), static_cast<BYTE>(value), LWA_COLORKEY);
    }

}




JSManager::JSManager()
{

    JSManager::functions = {
        // global functions
        { "Quit",               {   Quit,       TJSfunction::FUNC_GLOBAL    }},
        { "GetTitle",           {   GetTitle,   TJSfunction::FUNC_GLOBAL    }},
        { "Refresh",            {   Refresh,    TJSfunction::FUNC_GLOBAL    }},
        { "ToggleDesktopMgr",   {   ToggleDesktopMgr,   TJSfunction::FUNC_GLOBAL    }},

        // config functions
        { "SetStrValue",        {   config::SetStrValue,   TJSfunction::FUNC_CONFIG    }},
        { "SetIntValue",        {   config::SetIntValue,   TJSfunction::FUNC_CONFIG    }},
        { "GetStrValue",        {   config::GetStrValue,   TJSfunction::FUNC_CONFIG    }},
        { "GetIntValue",        {   config::GetIntValue,   TJSfunction::FUNC_CONFIG    }},

        // windows colors
        { "SetWindowsColor",        {   config::SetWindowsColor,   TJSfunction::FUNC_CONFIG    }},
        { "GetWindowsColor",        {   config::GetWindowsColor,   TJSfunction::FUNC_CONFIG    }},

        // startup
        { "ToggleStartup",        {   config::ToggleStartup,   TJSfunction::FUNC_CONFIG    }},

        //taskbar
        { "ChangeTaskTransparency",        {   config::ChangeTaskTransparency,   TJSfunction::FUNC_CONFIG    }},
        
    };
}

void JSManager::Initialize(CefRefPtr<CefBrowser> browser, CefRefPtr<CefV8Context> context)
{
    auto global = context->GetGlobal();

    auto confClass = CefV8Value::CreateObject(NULL, NULL);
    global->SetValue("config", confClass, V8_PROPERTY_ATTRIBUTE_NONE);


    for (auto& it : JSManager::functions)
    {
        auto func = CefV8Value::CreateFunction(it.first, this);

        switch (it.second.type)
        {
        case TJSfunction::FUNC_CONFIG:
            confClass->SetValue(it.first, func, V8_PROPERTY_ATTRIBUTE_NONE);
            continue;

        default:
            global->SetValue(it.first, func, V8_PROPERTY_ATTRIBUTE_NONE);
            continue;
        }
       
    }

}

bool JSManager::Execute(const CefString& name,
    CefRefPtr<CefV8Value> object,
    const CefV8ValueList& arguments,
    CefRefPtr<CefV8Value>& retval,
    CefString& exception)
{

    const auto &it = JSManager::functions.find(name.ToString().c_str());
    if (it != JSManager::functions.end())
    {
        it->second.function(retval, arguments, GLOBAL::GetBrowser());
        return true;
    }
    return false;
}

