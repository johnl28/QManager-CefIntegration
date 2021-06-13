#include "DesktopMgr.h"

#include <ctime>
#include "include/cef_v8.h"

#include "Manager.h"
#include "AppManager.h"
#include "framework.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/base/cef_bind.h"

BOOL CALLBACK EnumWindowFunc(HWND wmd, LPARAM param)
{
    HWND shWnd = FindWindowExW(wmd, nullptr, L"SHELLDLL_DefView", NULL);
    HWND sysWnd = FindWindowExW(shWnd, nullptr, L"SysListView32", NULL);

    if (!GLOBAL::GetAppMgr())
    {
        return false;
    }

    auto deskMgr = GLOBAL::GetAppMgr()->desktopMgr;

    if (!deskMgr->GetWindowPtr("Shell_TrayWnd"))
    {
        deskMgr->SetWindowPtr("Shell_TrayWnd", FindWindowExW(shWnd, nullptr, L"Shell_TrayWnd", NULL));
    }

    if (shWnd && sysWnd)
    {
        deskMgr->SetWindowPtr("WorkerW", wmd);
        deskMgr->SetWindowPtr("SHELLDLL_DefView", shWnd);
        deskMgr->SetWindowPtr("SysListView32", sysWnd);

        return FALSE;
    }
    return TRUE;
}


DesktopManager::DesktopManager()
{

}

DesktopManager::~DesktopManager()
{
    ToggleDesktopIcons(SW_SHOW);

    if (_thread)
    {
        GLOBAL::GetConfig()->SetValue("desk_mgr", 0);
        _thread->join();
        DESTROY_PTR(_thread);
        GLOBAL::GetConfig()->SetValue("desk_mgr", 1);
    }

}

HWND DesktopManager::GetWindowPtr(std::string name)
{
    auto it = m_wnd_map.find(name);
    if (it == m_wnd_map.end())
    {
        return nullptr;
    }
    return it->second;
}

void DesktopManager::_RefreshUIValues(bool click)
{
    SetJSValue("cur_time", _cur_time);
}


void DesktopManager::InitWorkerWnd()
{
    HWND workerWnd = FindWindow(L"Progman", NULL);
    HWND shWnd = FindWindowExW(workerWnd, nullptr, L"SHELLDLL_DefView", NULL);
    HWND sysWnd = FindWindowExW(shWnd, nullptr, L"SysListView32", NULL);
    m_wnd_map["Shell_TrayWnd"] = FindWindowExW(workerWnd, nullptr, L"Shell_TrayWnd", NULL);

    if (!sysWnd)
    {
        EnumChildWindows(GetDesktopWindow(), EnumWindowFunc, 0);
        return;
    }

    m_wnd_map["WorkerW"] = workerWnd;
    m_wnd_map["SHELLDLL_DefView"] = shWnd;
    m_wnd_map["SysListView32"] = sysWnd;
    //FindWindowExW(shWnd, nullptr, L"SysListView32", NULL);

}


void DesktopManager::ToggleDesktopIcons(int what)
{
    auto shWnd = GetWindowPtr("SysListView32");
    if (!shWnd) {
        InitWorkerWnd();
        return;
    }

    if (what && ::IsWindowVisible(shWnd))
        return;

    if (!what && !::IsWindowVisible(shWnd))
        return;

   // FindWindowExW(shWnd, nullptr, L"SysListView32", NULL)
    ShowWindow(shWnd, what);
}

bool DesktopManager::_CheckKeyState()
{
    if ((GetAsyncKeyState(GLOBAL::GetConfig()->GetInt("desk_mgr_key")) & 0x8000) && GetForegroundWindow() == GetWindowPtr("WorkerW"))
    {
        _cur_time = GLOBAL::GetConfig()->GetInt("desk_mgr_time");

        ToggleDesktopIcons(SW_SHOW);
        _RefreshUIValues(true);

        return true;
    }

    return false;
}

void DesktopManager::OnDeskUpdate()
{
    while (GLOBAL::GetConfig()->GetInt("desk_mgr"))
    {
        if (_CheckKeyState()) {
            Sleep(1000);
            continue;
        }

        if (_cur_time > GLOBAL::GetConfig()->GetInt("desk_mgr_time"))
        {
            _cur_time = GLOBAL::GetConfig()->GetInt("desk_mgr_time");
        }

        auto result = static_cast<double>(std::time(nullptr));

        if (result > _last_update && _cur_time)
        {
            --_cur_time;
            _last_update = result + 0.8;

            if (!_cur_time)
            {
                ToggleDesktopIcons(SW_HIDE);
            }

            _RefreshUIValues();
        }

        Sleep(50);
    }
}

void DesktopManager::Run()
{
    if (_thread) {
        return;
    }


    if (!GetWindowPtr("WorkerW"))
    {
        InitWorkerWnd();
    }

    _cur_time = GLOBAL::GetConfig()->GetInt("desk_mgr_time");

    GLOBAL::GetConfig()->SetValue("desk_mgr", 1);
    _thread = new std::thread([this]() { this->OnDeskUpdate(); });
}

void DesktopManager::QuitLopp()
{
    GLOBAL::GetConfig()->SetValue("desk_mgr", 0);
    if (_thread)
    {
        _thread->join();
        DESTROY_PTR(_thread);
    }
    ToggleDesktopIcons(SW_SHOW);

    _cur_time = GLOBAL::GetConfig()->GetInt("desk_mgr_time");

    _RefreshUIValues();
}

void DesktopManager::InitConfig()
{
    if (GLOBAL::GetConfig()->GetInt("desk_mgr"))
    {
        this->Run();
    }

    _cur_time = GLOBAL::GetConfig()->GetInt("desk_mgr_time");
    _RefreshUIValues();
}

#if 0
void DesktopUpdate()
{
    auto hwndPB = GetWindowPtr("DekstopMgrPB");
    while (!autoHideActive)
    {
        if (global::browser)
        {
            char buff[100] = {0};
            snprintf(buff, sizeof(buff), "OnDekstopManagerUpdate(%d, %d);", timeLeft, wait);
            global::browser->GetMainFrame()->ExecuteJavaScript(buff, global::browser->GetMainFrame()->GetURL(), 0);
        }

        if ((timeLeft == wait))
        {
            ToggleDesktopIcons(SW_HIDE);
            Sleep(1000);
            continue;
        }

        timeLeft += 1; // increase with 1 on each second

        Sleep(1000);
    }

}

void DesktopManager::CheckCursorState()
{

    while (!1)
    {
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && GetForegroundWindow() == GetWindowPtr("WorkerW") && timeLeft)
        {
            char buff[100] = { 0 };
            snprintf(buff, sizeof(buff), "OnDekstopManagerUpdate(0, %d);", wait);
            global::browser->GetMainFrame()->ExecuteJavaScript(buff, global::browser->GetMainFrame()->GetURL(), 0);
            timeLeft = 0;
            ToggleDesktopIcons(SW_SHOW);
        }


        Sleep(10);
    }

}


void DesktopManager::OnDeskUpdate()
{
}

void DesktopManager::Run()
{
    if (!GetWindowPtr("WorkerW"))
    {
        InitWorkerWnd();
    }

    SpawnDesktopThreads();


}

void ToggleAutoHideIcons()
{

    autoHideActive = !autoHideActive;

    if (autoHideActive)
    {
        SpawnDesktopThreads();
    }
    else
    {
        delete threads["dekstop_update"];
        threads["dekstop_update"] = nullptr;
        delete threads["cursor_state"];
        threads["dekstop_update"] = nullptr;
        ToggleDesktopIcons(SW_SHOW);
        timeLeft = 0;
    }

}

#endif

