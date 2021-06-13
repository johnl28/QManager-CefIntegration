#include "DesktopMgr.h"
#include "AppManager.h"
#include "Manager.h"

APP_MGR::APP_MGR()
{
 
}

void APP_MGR::Init()
{
    desktopMgr = new DesktopManager;
    desktopMgr->InitWorkerWnd();
    desktopMgr->InitConfig();
}

APP_MGR::~APP_MGR()
{
    DESTROY_PTR(desktopMgr);
}

