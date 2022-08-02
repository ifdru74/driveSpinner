#include <Windows.h>
#include <WinBase.h>
#include "pch.h"
#include "diagData.h"
#include "WinServiceHelper.h"
constexpr int BUFFER_SIZE = 4096;
WinServiceHelper::WinServiceHelper(const STDSTRING& serviceName) : 
    szServiceName(NULL), 
    servicePath(), 
    hMod(NULL), 
    hSCManager(NULL), 
    hService(NULL)
{
	memset(&ServiceStatus, 0, sizeof(SERVICE_STATUS));
	memset(&hStatus, 0, sizeof(SERVICE_STATUS_HANDLE));
    szBuf = new TCHAR[BUFFER_SIZE + 1];
    szServiceName = new TCHAR[serviceName.length() + 1];
}

WinServiceHelper::~WinServiceHelper()
{
    if (szServiceName != NULL)
    {
        delete[] szServiceName;
        szServiceName = NULL;
    }
    if (szBuf != NULL)
    {
        delete[] szBuf;
        szBuf = NULL;
    }
    if (hSCManager != NULL)
    {
        CloseServiceHandle(hSCManager);
    }
}

bool WinServiceHelper::fillModuleInfo(LPCTSTR szModName)
{
    if (hMod == NULL)
    {
        hMod = GetModuleHandle(NULL);
        if (hMod != NULL && szBuf != NULL)
        {
            memset(szBuf, 0, BUFFER_SIZE * sizeof(TCHAR));
            GetModuleFileName(hMod, szBuf, BUFFER_SIZE);
            servicePath.assign(szBuf);
        }
    }
    return (hMod != NULL && szBuf != NULL);
}
bool WinServiceHelper::install()
{
    if (!(hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE))) {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable open Service Control Manager") << std::endl;
        return false;
    }
    if (!fillModuleInfo())
    {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable to fill module info") << std::endl;
        return false;
    }
    if (!(hService = CreateService(
        hSCManager,
        szServiceName,
        szServiceName,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        servicePath.c_str(),
        NULL, NULL, NULL, NULL, NULL))) {
        int err = GetLastError();
        switch (err) {
        case ERROR_ACCESS_DENIED:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_ACCESS_DENIED") << std::endl;
            break;
        case ERROR_CIRCULAR_DEPENDENCY:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_CIRCULAR_DEPENDENCY") << std::endl;
            break;
        case ERROR_DUPLICATE_SERVICE_NAME:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_DUPLICATE_SERVICE_NAME") << std::endl;
            break;
        case ERROR_INVALID_HANDLE:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_INVALID_HANDLE") << std::endl;
            break;
        case ERROR_INVALID_NAME:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_INVALID_NAME") << std::endl;
            break;
        case ERROR_INVALID_PARAMETER:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_INVALID_PARAMETER") << std::endl;
            break;
        case ERROR_INVALID_SERVICE_ACCOUNT:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_INVALID_SERVICE_ACCOUNT") << std::endl;
            break;
        case ERROR_SERVICE_EXISTS:
            STDCOUT << print::diagData::timeStamp << _T("Error: ERROR_SERVICE_EXISTS") << std::endl;
            break;
        default:
            STDCOUT << print::diagData::timeStamp << _T("Error: Undefined") << std::endl;
        }
        CloseServiceHandle(hSCManager);
        return false;
    }
    CloseServiceHandle(hService);

    CloseServiceHandle(hSCManager);
    STDCOUT << print::diagData::timeStamp << _T("Service '") << szServiceName << _T("' installed successfully")<< std::endl;
    return true;
}

bool WinServiceHelper::remove()
{
    if (!(hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable open Service Control Manager") << std::endl;
        return -1;
    }
    if (!(hService = OpenService(hSCManager, szServiceName, SERVICE_STOP | DELETE))) {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable remove service") << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    DeleteService(hService);
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    STDCOUT << print::diagData::timeStamp << _T("Service '") << szServiceName << _T("' removed successfully") << std::endl;
    return true;
}

bool WinServiceHelper::start()
{
    bool bRet = false;
    if (hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE))
    {
        if (hService = OpenService(hSCManager, szServiceName, SERVICE_START))
        {
            if (StartService(hService, 0, NULL)) {
                STDCOUT << print::diagData::timeStamp << _T("Service '")  << szServiceName << _T("' started successfully") << std::endl;
                bRet = true;
            }
            else
            {
                STDCOUT << print::diagData::timeStamp << _T("Error: Unable to start service '") << szServiceName << _T("'") << std::endl;
            }
            CloseServiceHandle(hService);
        }
        else
        {
            STDCOUT << print::diagData::timeStamp << _T("Error: Unable to open service '") << szServiceName << _T("'") << std::endl;
        }
        CloseServiceHandle(hSCManager);
    }
    else
    {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable open Service Control Manager") << std::endl;
    }
    return bRet;
}

bool WinServiceHelper::stop()
{
    bool bRet = false;
    if (hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE))
    {
        if (hService = OpenService(hSCManager, szServiceName, SERVICE_START))
        {
            if (ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus)) {
                STDCOUT << print::diagData::timeStamp << _T("Service '") << szServiceName << _T("' stopped successfully") << std::endl;
                bRet = true;
            }
            else
            {
                STDCOUT << print::diagData::timeStamp << _T("Error: Unable to start service '") << szServiceName << _T("'") << std::endl;
            }
            CloseServiceHandle(hService);
        }
        else
        {
            STDCOUT << print::diagData::timeStamp << _T("Error: Unable to open service '") << szServiceName << _T("'") << std::endl;
        }
        CloseServiceHandle(hSCManager);
    }
    else
    {
        STDCOUT << print::diagData::timeStamp << _T("Error: Unable open Service Control Manager") << std::endl;
    }
    return bRet;
}

bool WinServiceHelper::initServiceStatus(LPHANDLER_FUNCTION handler)
{
    ServiceStatus.dwServiceType      = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState     = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode    = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint       = 0;
    ServiceStatus.dwWaitHint         = 0;
    if (handler == NULL)
    {
        if (controlHandler != NULL)
        {
            hStatus = RegisterServiceCtrlHandler(szServiceName, controlHandler);

        }
    }
    else
    {
        hStatus = RegisterServiceCtrlHandler(szServiceName, handler);
    }
    return (hStatus != (SERVICE_STATUS_HANDLE)0);
}

void WinServiceHelper::setStatusAndCode(DWORD dsStatus, DWORD dwCode)
{
    ServiceStatus.dwCurrentState = dsStatus;
    ServiceStatus.dwWin32ExitCode = dwCode;
    SetServiceStatus(hStatus, &ServiceStatus);

}

const DWORD WinServiceHelper::getStatus() const
{
    return ServiceStatus.dwCurrentState;
}