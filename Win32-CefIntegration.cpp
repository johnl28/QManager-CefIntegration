/*
Author: Johnl
Email: ionutcriste28@gmail.com
Date: 28.05.2021


Useful links:
https://google.github.io/styleguide/cppguide.html

*/

#include "framework.h"
#include "Win32-CefIntegration.h"
#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_browser.h"
#include "app.h"
#include "handler.h"
#include "Manager.h"
#include <thread>
#include "Config.h"




// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32CEFINTEGRATION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32CEFINTEGRATION));

    MSG msg;


    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        CefDoMessageLoopWork();
    }

    return (int) msg.wParam;
}

//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32CEFINTEGRATION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   CefEnableHighDPISupport();

   CefMainArgs main_args(hInstance);
   GLOBAL::get().Init(hInstance);
   GLOBAL::get().InitConfig();


   auto exit_code = CefExecuteProcess(main_args, GLOBAL::get().cefapp_, nullptr);
   if (exit_code >= 0) {
       return exit_code;
   }

   if (GLOBAL::GetConfig()->GetInt("dev-mode-cfg"))
   {
       InitConsole("Main Process Console");
   }


   GLOBAL::get().window_ = CreateWindowW(szWindowClass, szTitle, WS_POPUP,
        GetSystemMetrics(SM_CXSCREEN) / 2 - 400, 200, 500, 450, nullptr, nullptr, hInstance, nullptr);
   

   if (!GLOBAL::GetWindow()) {
      return FALSE;
   }

   //ShowWindow(global::hWnd, 0);
   UpdateWindow(GLOBAL::GetWindow());

   GLOBAL::GetCefApp()->Init(GLOBAL::GetWindow(), hInst, main_args);
   GLOBAL::get().InitShellIcon();

   return TRUE;
}



//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT lpClickPoint;

    switch (message)
    {
    case WM_USER_SHELLICON:
    {
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
        {
            UINT uFlag = MF_BYPOSITION | MF_STRING;
            GetCursorPos(&lpClickPoint);
            auto hPopMenu = CreatePopupMenu();
            InsertMenu(hPopMenu, 0xFFFFFFFF, uFlag, IDM_OPEN, _T("Open"));

            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, 3, _T("SEP"));
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, _T("Exit"));

            SetForegroundWindow(hWnd);
            TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
            return TRUE;
        }
        case WM_LBUTTONDOWN:
        {
            if (!::IsWindowVisible(hWnd))
            {
                GLOBAL::GetBrowser()->Reload();
            }

            //UpdateWindow(hWnd);
            SetForegroundWindow(hWnd);
            BringWindowToTop(hWnd);
        }
        return 0;

        }
    }
    break;


    case WM_NCHITTEST: 
        {
            LRESULT hit = DefWindowProc(hWnd, message, wParam, lParam);
            if (hit == HTCLIENT) 
            {
                POINTS points = MAKEPOINTS(lParam);
                POINT point = { points.x, points.y };
                ::ScreenToClient(hWnd, &point);
                if (::PtInRegion(GLOBAL::get().draggable_region_, point.x, point.y)) 
                {
                    return HTCAPTION;
                }
            }
            return hit;
        }
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_EXIT:
                ShutDown();
                break;
            case IDM_OPEN:
                GLOBAL::GetBrowser()->Reload();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
    {
        ShutDown();
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void InitConsole(const char* name)
{
    AllocConsole();

    AttachConsole(GetCurrentProcessId());
    FILE* pNewStdout = nullptr;
    FILE* pNewStderr = nullptr;
    FILE* pNewStdin = nullptr;

    ::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
    ::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
    ::freopen_s(&pNewStdin, "CONIN$", "r", stdin);

    SetConsoleTitleA(name);
}


void ShutDown()
{
    printf("SHUTDOWN CALLED ! \n");
    ShowWindow(GLOBAL::GetWindow(), SW_HIDE);
    GLOBAL::get().Destroy();

    FreeConsole();

    CefShutdown();
    PostQuitMessage(0);
}


