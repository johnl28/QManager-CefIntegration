// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#include "handler.h"
#include "include/cef_app.h"



// Implement application-level callbacks for the browser process.
class MainApp : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
{
private:

    HWND _parentWnd = nullptr;
    HINSTANCE _instance = nullptr;

public:
     MainApp();

     // CefApp methods:
     virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE  { return this; }
     virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

    // CefBrowserProcessHandler methods:
    virtual void OnContextInitialized() OVERRIDE;

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefV8Context> context) OVERRIDE;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) OVERRIDE;

    virtual void OnBeforeCommandLineProcessing(
        const CefString& process_type,
        CefRefPtr<CefCommandLine> command_line) OVERRIDE;

    void Init(HWND parent, HINSTANCE instance, CefMainArgs &main_args);


private:
    void ShowPopLoading(CefBrowserSettings browser_settings, CefRefPtr<SimpleHandler> handler);
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(MainApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
