// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "handler.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "app.h"
#include "Manager.h"
#include "AppManager.h"


namespace 
{
    SimpleHandler* g_instance = nullptr;

    // Returns a data: URI with the specified contents.
    std::string GetDataURI(const std::string& data, const std::string& mime_type) 
    {
        return "data:" + mime_type + ";base64," + CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
    }

}  // namespace

SimpleHandler::SimpleHandler(bool use_views) : use_views_(use_views), is_closing_(false) 
{
    DCHECK(!g_instance);
    g_instance = this;
}

SimpleHandler::~SimpleHandler() 
{
    g_instance = nullptr;
}

// static
SimpleHandler* SimpleHandler::GetInstance() 
{
    return g_instance;
}


void SimpleHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    int httpStatusCode)
{

    if (!frame->IsMain()) {
        return;
    }

    //
    if (!GLOBAL::GetBrowser())
    {
        GLOBAL::get().browser_ = browser;

        GLOBAL::get().appmgr_ = new APP_MGR;
        GLOBAL::get().appmgr_->Init();
    }
       
    // SHOW THE WINDOW WHEN THE MAIN FRAME IS LOADED
    if (!IsWindowVisible(GLOBAL::GetWindow()))
    {
        ShowWindow(GLOBAL::GetWindow(), SW_SHOW);
        UpdateWindow(GLOBAL::GetWindow());
        BringWindowToTop(GLOBAL::GetWindow());
    }

    printf("Main UI loaded! \n");
}


void SimpleHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser,
    TerminationStatus status)
{
    printf("RENDER PROCESS ENDED, CODE %d \n", status);
    browser->Reload();
}

bool SimpleHandler::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefProcessId source_process,
    CefRefPtr<CefProcessMessage> message)
{

    const auto name = message->GetName();
    printf("PROCESS MESSAGE RECEIVED NAME: %s\n", name.ToString().c_str());

    if (name == "SYNC_CONF_INT")
    {
        printf("CONF SYNC REC %s, %d \n", message->GetArgumentList()->GetString(0).ToString().c_str(), message->GetArgumentList()->GetInt(1));
        GLOBAL::GetConfig()->SetValue(message->GetArgumentList()->GetString(0).ToString().c_str(), message->GetArgumentList()->GetInt(1));
        return true;
    }

    const auto arg = message->GetArgumentList()->GetInt(0);
    if (signal::ProcessSignal(arg)) {
        return true;
    }

    return false;
}


void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser) 
{
    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser) 
{
    CEF_REQUIRE_UI_THREAD();

    if (browser_list_.size() == 1) 
    {
        // Set a flag to indicate that the window close should be allowed.
        is_closing_ = true;
    }

     return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser) 
{
     CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    auto bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) 
    {
        if ((*bit)->IsSame(browser)) 
        {
            browser_list_.erase(bit);
            break;
        }
    }

    if(browser_list_.empty()) 
    {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
                                CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode,
                                const CefString& errorText,
                                const CefString& failedUrl) 
{
    CEF_REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED)
        return;

    // Display a load error message using a data: URI.
     std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
        "<h2>Failed to load URL "
        << std::string(failedUrl) << " with error " << std::string(errorText)
        << " (" << errorCode << ").</h2></body></html>";

     frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
    bool isLoading,
    bool canGoBack,
    bool canGoForward)
{

}

void SimpleHandler::CloseAllBrowsers(bool force_close) 
{
    CEF_REQUIRE_UI_THREAD();

    if (!CefCurrentlyOn(TID_UI)) 
    {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
        return;
    }

    if (browser_list_.empty())
        return;

    for (auto it = browser_list_.begin(); it != browser_list_.end(); ++it)
    {
        (*it)->GetHost()->CloseBrowser(force_close);
    }
       
}

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();

    if (use_views_)
    {
        // Set the title of the window using the Views framework.
        CefRefPtr<CefBrowserView> browser_view = CefBrowserView::GetForBrowser(browser);
        if (browser_view)
        {
            CefRefPtr<CefWindow> window = browser_view->GetWindow();
            if (window)
                window->SetTitle(title);
        }
    }
    else
    {
        // Set the title of the window using platform APIs.
        PlatformTitleChange(browser, title);
    }
}

//Disable context menu
//Define below two functions to essentially do nothing, overwriting defaults
//See change in simple_handler.h
void SimpleHandler::OnBeforeContextMenu(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    CefRefPtr<CefMenuModel> model) 
{
    CEF_REQUIRE_UI_THREAD();

    model->Clear();
}

bool SimpleHandler::OnContextMenuCommand(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    CefRefPtr<CefContextMenuParams> params,
    int command_id,
    EventFlags event_flags) 
{
    CEF_REQUIRE_UI_THREAD();

   // MessageBox(browser->GetHost()->GetWindowHandle(), L"The requested action is not supported", L"Unsupported Action", MB_OK | MB_ICONINFORMATION);
    return false;
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{

    CEF_REQUIRE_UI_THREAD();
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();

    if (hwnd)
    {
        SetWindowText(hwnd, std::wstring(title).c_str());
    }

}

// ************************************/


LPCWSTR kParentWndProc = L"CefParentWndProc";
LPCWSTR kDraggableRegion = L"CefDraggableRegion";

LRESULT CALLBACK SubclassedWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) 
{
    WNDPROC hParentWndProc = reinterpret_cast<WNDPROC>(::GetPropW(hWnd, kParentWndProc));
    HRGN hRegion = reinterpret_cast<HRGN>(::GetPropW(hWnd, kDraggableRegion));

    if (message == WM_NCHITTEST) 
    {
        LRESULT hit = CallWindowProc(hParentWndProc, hWnd, message, wParam, lParam);
        if (hit == HTCLIENT) {
            POINTS points = MAKEPOINTS(lParam);
            POINT point = { points.x, points.y };
            ::ScreenToClient(hWnd, &point);
            if (::PtInRegion(hRegion, point.x, point.y)) {
                // Let the parent window handle WM_NCHITTEST by returning HTTRANSPARENT
                // in child windows.
                return HTTRANSPARENT;
            }
        }
        return hit;
    }

    return CallWindowProc(hParentWndProc, hWnd, message, wParam, lParam);
}


void SubclassWindow(HWND hWnd, HRGN hRegion) 
{
    HANDLE hParentWndProc = ::GetPropW(hWnd, kParentWndProc);
    if (hParentWndProc) 
    {
        return;
    }

    SetLastError(0);
    LONG_PTR hOldWndProc = SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassedWindowProc));

    if (hOldWndProc == 0 && GetLastError() != ERROR_SUCCESS) 
    {
        return;
    }

    ::SetPropW(hWnd, kParentWndProc, reinterpret_cast<HANDLE>(hOldWndProc));
    ::SetPropW(hWnd, kDraggableRegion, reinterpret_cast<HANDLE>(hRegion));

}

void UnSubclassWindow(HWND hWnd) 
{
    LONG_PTR hParentWndProc = reinterpret_cast<LONG_PTR>(::GetPropW(hWnd, kParentWndProc));

    if (hParentWndProc) 
    {
        LONG_PTR hPreviousWndProc =  SetWindowLongPtr(hWnd, GWLP_WNDPROC, hParentWndProc);
        ALLOW_UNUSED_LOCAL(hPreviousWndProc);
        DCHECK_EQ(hPreviousWndProc, reinterpret_cast<LONG_PTR>(SubclassedWindowProc));
    }

    ::RemovePropW(hWnd, kParentWndProc);
    ::RemovePropW(hWnd, kDraggableRegion);
}

BOOL CALLBACK SubclassWindowsProc(HWND hwnd, LPARAM lParam) 
{
    SubclassWindow(hwnd, reinterpret_cast<HRGN>(lParam));
    return TRUE;
}

BOOL CALLBACK UnSubclassWindowsProc(HWND hwnd, LPARAM lParam) 
{
    UnSubclassWindow(hwnd);
    return TRUE;
}


void SimpleHandler::OnDraggableRegionsChanged(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefFrame> frame,
    const std::vector<CefDraggableRegion>& regions)
{
    CEF_REQUIRE_UI_THREAD();


    // Reset draggable region.
    ::SetRectRgn(GLOBAL::get().draggable_region_, 0, 0, 0, 0);


    for (auto it = regions.begin(); it != regions.end(); ++it)
    {
        HRGN region = ::CreateRectRgn(it->bounds.x, it->bounds.y,
            it->bounds.x + it->bounds.width,
            it->bounds.y + it->bounds.height);

        ::CombineRgn(GLOBAL::get().draggable_region_, GLOBAL::get().draggable_region_, region,
            it->draggable ? RGN_OR : RGN_DIFF);

        ::DeleteObject(region);
    }


    auto hwnd_ = GLOBAL::GetWindow();

    WNDENUMPROC proc = !regions.empty() ? SubclassWindowsProc : UnSubclassWindowsProc;
    ::EnumChildWindows(hwnd_, proc, reinterpret_cast<LPARAM>(GLOBAL::get().draggable_region_));

}

