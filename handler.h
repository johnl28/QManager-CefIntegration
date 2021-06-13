// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_

#include "include/cef_v8.h"
#include "include/cef_client.h"
#include "include/cef_render_process_handler.h"
#include <list>


class SimpleHandler : public CefClient,
                      public CefDisplayHandler,
                      public CefLifeSpanHandler,
                      public CefLoadHandler,
                      public CefJSDialogHandler,
                      public CefContextMenuHandler,
                      public CefDragHandler,
                      public CefRequestHandler
{

public:
    explicit SimpleHandler(bool use_views);
    ~SimpleHandler();

    // Provide access to the single global instance of this object.
    static SimpleHandler* GetInstance();

    // CefClient methods:
    virtual CefRefPtr<CefDisplayHandler>        GetDisplayHandler()            OVERRIDE { return this; }
    virtual CefRefPtr<CefLifeSpanHandler>       GetLifeSpanHandler()           OVERRIDE { return this; }
    virtual CefRefPtr<CefLoadHandler>           GetLoadHandler()               OVERRIDE { return this; }
    virtual CefRefPtr<CefJSDialogHandler>       GetJSDialogHandler()           OVERRIDE { return this; }
    virtual CefRefPtr<CefContextMenuHandler>    GetContextMenuHandler()        OVERRIDE { return this; }
    virtual CefRefPtr<CefDragHandler>           GetDragHandler()               OVERRIDE { return this; }
    virtual CefRefPtr<CefRequestHandler>        GetRequestHandler()            OVERRIDE { return this; }
    
    // CefDisplayHandler methods:
     virtual void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) OVERRIDE;

    // CefLifeSpanHandler methods:
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser)          OVERRIDE;
    virtual bool DoClose(CefRefPtr<CefBrowser> browser)                 OVERRIDE;
    virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser)           OVERRIDE;
    virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int httpStatusCode) OVERRIDE;

    virtual void OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
        bool isLoading,
        bool canGoBack,
        bool canGoForward) OVERRIDE;

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) OVERRIDE;

    // CefLoadHandler methods:
    virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
                           CefRefPtr<CefFrame> frame,
                           ErrorCode errorCode,
                           const CefString& errorText,
                           const CefString& failedUrl) OVERRIDE;


    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
        TerminationStatus status) OVERRIDE;


    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    // CefContextMenuHandler methods
    virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        CefRefPtr<CefMenuModel> model) OVERRIDE;

    virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefContextMenuParams> params,
        int command_id,
        EventFlags event_flags) OVERRIDE;


    virtual void OnDraggableRegionsChanged(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        const std::vector<CefDraggableRegion>& regions) OVERRIDE;


    bool IsClosing() const { return is_closing_; }

private:
    // Platform-specific implementation.
    void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title);

    // True if the application is using the Views framework.
    const bool use_views_;

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    bool is_closing_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleHandler);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_HANDLER_H_
