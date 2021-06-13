
#ifndef _DESK_MGR_H_
#define _DESK_MGR_H_
#include <map>
#include <vector>
#include <thread>
#include <string>
#include <windows.h>
#include <stdint.h>

#include "include/cef_frame.h"


class DesktopManager
{
public:
	DesktopManager();
	~DesktopManager();

private:
	std::thread *_thread = nullptr;

	std::map<std::string, HWND> m_wnd_map;

	double _last_update = 0.0f;
	uint8_t _cur_time = 0;

	void ToggleDesktopIcons(int what);
	bool _CheckKeyState();
	void _RefreshUIValues(bool click = false);

public:
	void InitWorkerWnd();
	HWND GetWindowPtr(std::string name);
	void SetWindowPtr(std::string name, HWND wnd) { m_wnd_map[name] = wnd; };

	void OnDeskUpdate();
	void QuitLopp();
	void Run();

	void InitConfig();
};




#endif