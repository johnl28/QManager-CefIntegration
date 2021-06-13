// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "app.h"

#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "handler.h"
#include "JSManager.h"
#include "Manager.h"
#include "Win32-CefIntegration.h"

MainApp::MainApp()
{
    
}

void MainApp::Init(HWND parent, HINSTANCE instance, CefMainArgs& main_args)
{
    CEF_REQUIRE_UI_THREAD();

    _parentWnd = parent;
    _instance = instance;

    CefSettings settings;
    settings.multi_threaded_message_loop = false;
    //settings.log_severity = LOGSEVERITY_DISABLE;
    settings.remote_debugging_port = 8080;
    //settings.windowless_rendering_enabled = true;
    settings.no_sandbox = true;
    printf("CEF INITIALIZATION!\n");

    CefInitialize(main_args, settings, this, nullptr);
};



void MainApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefV8Context> context)
{
    CEF_REQUIRE_RENDERER_THREAD();

    // Create an instance of my CefV8Handler object.
    if (!GLOBAL::GetJSHandler())
    {
        if (GLOBAL::GetConfig()->GetInt("dev-mode-cfg"))
        {
            InitConsole("Render Process Console");
        }

        GLOBAL::get().InitConfig();

        GLOBAL::get().jshandler_ = new JSManager();
        GLOBAL::get().browser_ = browser;
    }

    if (frame->IsMain())
    {
        GLOBAL::get().context_ = context;
        printf("Main context created! \n");
    }

    GLOBAL::GetJSHandler()->Initialize(browser, context);

}

void MainApp::ShowPopLoading(CefBrowserSettings browser_settings, CefRefPtr<SimpleHandler> handler)
{

    CefWindowInfo window_info;

    window_info.SetAsPopup(_parentWnd, "TEST");
    CefBrowserHost::CreateBrowser(window_info, handler, "https://www.youtube.com/watch?v=IxyoU9GU3Wc&ab_channel=RUSTAGERUSTAGEVerified", browser_settings, nullptr, nullptr);
}

void MainApp::OnContextInitialized()
{

    CEF_REQUIRE_UI_THREAD();

    // SimpleHandler implements browser-level callbacks.
    CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));

    // Specify CEF browser settings here.
    CefBrowserSettings browser_settings;
    browser_settings.javascript = STATE_ENABLED;
   //browser_settings.webgl = STATE_ENABLED;
    browser_settings.universal_access_from_file_urls = STATE_ENABLED;
    browser_settings.file_access_from_file_urls = STATE_ENABLED;
    browser_settings.webgl = STATE_ENABLED;
    browser_settings.windowless_frame_rate = 60;
    CefWindowInfo window_info;

    RECT htmlRect;
    WINDOWINFO wndInfo;
    GetWindowInfo(_parentWnd, &wndInfo);
    GetWindowRect(_parentWnd, &htmlRect);

    htmlRect.left -= wndInfo.rcClient.left;
    htmlRect.right -= wndInfo.rcClient.left;
    htmlRect.top -= wndInfo.rcClient.top;
    htmlRect.bottom -= wndInfo.rcClient.top;

    window_info.SetAsChild(_parentWnd, htmlRect);
    //CefBrowserHost::CreateBrowser(window_info, handler, "https://github.com/chromiumembedded/cef", browser_settings, nullptr, nullptr);
    // Create the first browser window.'

    CefBrowserHost::CreateBrowser(window_info, handler, GLOBAL::GetConfig()->GetString("link").c_str(), browser_settings, nullptr, nullptr);
  //  ShowPopLoading(browser_settings, handler);
}

void MainApp::OnBeforeCommandLineProcessing(const CefString& ProcessType, CefRefPtr<CefCommandLine> CommandLine)
{

    CEF_REQUIRE_UI_THREAD();

    CommandLine->AppendSwitch("allow-running-insecure-content");
    CommandLine->AppendSwitch("enable-media-stream");
    CommandLine->AppendSwitch("disable-web-security");
    CommandLine->AppendSwitch("enable-local-file-accesses");
    CommandLine->AppendSwitch("no-proxy-server");
    CommandLine->AppendSwitch("disable-gpu-compositing");

    if (!GLOBAL::GetConfig()->GetInt("render-cfg"))
    {
        CommandLine->AppendSwitch("disable-gpu");
        printf("GPU HARDWARE ACCELERATION DISABLED \n");
    } else {
        printf("GPU HARDWARE ACCELERATION ENABLED \n");
    }

    printf("COMMAND LINE PRROCESS CALLED\n");
    
}


bool MainApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{
    CEF_REQUIRE_RENDERER_THREAD();

    const auto name = message->GetName();

    printf("PROCESS MESSAGE RECEIVED NAME: %s\n", name.ToString().c_str());

    if (!CefCurrentlyOn(TID_RENDERER) || !frame) {
        return false;
    }

    auto args = message->GetArgumentList();

    auto context = GLOBAL::GetGlobalContext();
    if (!context)
        return false;

    if (name == "JS_VAL_SYNC_INT")
    {
        context->Enter();
        context->GetGlobal()->SetValue(args->GetString(0), CefV8Value::CreateInt(args->GetInt(1)), V8_PROPERTY_ATTRIBUTE_NONE);
        context->Exit();
    }
    else if (name == "JS_VAL_SYNC_STR")
    {
        context->Enter();
        context->GetGlobal()->SetValue(args->GetString(0), CefV8Value::CreateString(args->GetString(1)), V8_PROPERTY_ATTRIBUTE_NONE);
        context->Exit();
    }
    else
    {
        return false;
    }

    return true;
}

