#include <Windows.h>
#include <WinBase.h>
#include "pch.h"
#include "diagData.h"
#include "AppModule.h"
#include "WinServiceHelper.h"

WinServiceHelper AppModule::svc(_T("DriveSpin"));
SERVICE_TABLE_ENTRY AppModule::ServiceTable[1];

void AppModule::ServiceMain(int argc, char** argv)
{
    if (svc.initServiceStatus())
    {
        if (initialize())
        {
            svc.setStatusAndCode(SERVICE_RUNNING);
            while (svc.getStatus() == SERVICE_RUNNING)
            {

            }
        }
        svc.setStatusAndCode(SERVICE_STOPPED, -1);
    }
}

bool AppModule::initialize()
{
}

int AppModule::doService(int argc, char** argv)
{
    AppModule::ServiceTable[0].lpServiceName = svc.getName();
    AppModule::ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)AppModule::ServiceMain;

    StartServiceCtrlDispatcher(ServiceTable);

}
