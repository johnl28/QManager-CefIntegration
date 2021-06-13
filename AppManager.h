#ifndef _HEADER_APP_MANAGER_
#define _HEADER_APP_MANAGER_
#include "DesktopMgr.h"



class APP_MGR
{
public:
	APP_MGR();
	~APP_MGR();

// APP POINTERS
public:
	void Init();
	DesktopManager* desktopMgr = nullptr;
};

#endif // !_HEADER_APP_MANAGER_



