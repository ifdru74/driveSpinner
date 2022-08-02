#include "pch.h"
#include <Windows.h>
#include "WinServiceModule.h"

WinServiceModule* WinServiceModule::pThis = nullptr;

WinServiceModule::WinServiceModule(LPCTSTR szSvcName, LPCTSTR szSvcPath) :
	hSCManager(nullptr), hService(nullptr), pServiceTable(nullptr),
	serviceStatus(), dwThreadID(0), m_bService(FALSE), serviceStatusHandle(0), szServiceName(nullptr),
	sServicePath(), pLog(NULL), errTxt(), hServiceStopEvent(nullptr)
{
	if (!szSvcName || !szSvcPath)
		throw std::bad_alloc();
	pServiceTable = (SERVICE_TABLE_ENTRY*)calloc(2, sizeof(SERVICE_TABLE_ENTRY));
	if (pServiceTable == nullptr)
		throw std::bad_alloc();
	size_t nLen = _tcslen(szSvcName) + 1;
	szServiceName = (TCHAR*)calloc(nLen,sizeof(TCHAR));
	if(szServiceName==nullptr)
		throw std::bad_alloc();
	_tcsncpy_s(szServiceName, nLen, szSvcName, nLen-1);
	sServicePath.assign(szSvcPath);
}

WinServiceModule::~WinServiceModule()
{
	if (szServiceName)
	{
		free(szServiceName);
		szServiceName = nullptr;
	}
	if (pServiceTable)
	{
		free(pServiceTable);
		pServiceTable = nullptr;
	}
	pServiceTable = nullptr;
	if (pLog)
	{
		closeLogFile(pLog);
		pLog = NULL;
	}
	if (hService != nullptr)
	{
		closeServiceHandle(hService);
		hService = nullptr;
	}
	if (hSCManager)
	{
		closeServiceHandle(hSCManager);
		hSCManager = nullptr;
	}
	if (hServiceStopEvent != nullptr)
	{
		CloseHandle(hServiceStopEvent);
		hServiceStopEvent = nullptr;
	}
}

void WINAPI WinServiceModule::_Handler(DWORD dwOpcode)
{
	switch (dwOpcode)
	{
	case SERVICE_CONTROL_STOP:
		WinServiceModule::pThis->addLogMessage(_T("Stopped."));

		WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = 0;
		WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_STOPPED;
		WinServiceModule::pThis->setServiceStatus();
		return;

	case SERVICE_CONTROL_SHUTDOWN:
		WinServiceModule::pThis->addLogMessage(_T("Shutdown."));

		WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = 0;
		WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_STOPPED;
		WinServiceModule::pThis->setServiceStatus();
		return;

	default:
		break;
	}

	WinServiceModule::pThis->setServiceStatus();

	return;
}

int WinServiceModule::addLogMessage(const TCHAR* szMsg, int iSev, int iCode)
{
	logPrintf(pLog, iSev, iCode, _T("%s\n"), szMsg);
	return EXIT_SUCCESS;
}
void WINAPI WinServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	int error;
	int i = 0;
	Sleep(20000);
	STDSTRING sLogPath = WinServiceModule::pThis->sServicePath;
	size_t pos = sLogPath.find(_T(".exe"));
	if (pos != std::string::npos)
	{
		sLogPath.replace(pos, 4, _T(".log"));
		WinServiceModule::pThis->openLog(sLogPath.c_str());
	}
	else
	{
		sLogPath = _T("D:\\Projects\\C_CPP\\driveSpinner\\x64\\Debug\\WinServiceModule.log");
		WinServiceModule::pThis->openLog(sLogPath.c_str());
	}
	logPrintf(WinServiceModule::pThis->pLog, SEV_INFO, 2, _T("Service is starting\n"));
	WinServiceModule::pThis->serviceStatusHandle = RegisterServiceCtrlHandler((LPCTSTR)WinServiceModule::pThis->szServiceName, (LPHANDLER_FUNCTION)_Handler);
	if (WinServiceModule::pThis->serviceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		logPrintf(WinServiceModule::pThis->pLog, SEV_ERROR, 1, _T("Unable to register service status handle\n"));
	}
	WinServiceModule::pThis->serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	WinServiceModule::pThis->serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = 0;
	WinServiceModule::pThis->serviceStatus.dwServiceSpecificExitCode = 0;
	WinServiceModule::pThis->serviceStatus.dwCheckPoint = 0;
	WinServiceModule::pThis->serviceStatus.dwWaitHint = 0;
	WinServiceModule::pThis->setServiceStatus();
	error = WinServiceModule::pThis->initService();
	if (error != EXIT_SUCCESS)
	{
		WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = S_FALSE;
		WinServiceModule::pThis->serviceStatus.dwCheckPoint = 0;
		WinServiceModule::pThis->serviceStatus.dwWaitHint = 0;
		WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_STOPPED;
	}
	else
	{
		WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = S_OK;
		WinServiceModule::pThis->serviceStatus.dwCheckPoint = 0;
		WinServiceModule::pThis->serviceStatus.dwWaitHint = 0;
		WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_RUNNING;

	}
	bool bMain = true;
	while (bMain)
	{
		if (WinServiceModule::pThis->hServiceStopEvent == nullptr)
			break;
		switch (WaitForSingleObject(WinServiceModule::pThis->hServiceStopEvent, 1000))
		{
		case WAIT_TIMEOUT:
			continue;
		case WAIT_OBJECT_0:
			bMain = false;
			break;
		}
	}
	logPrintf(WinServiceModule::pThis->pLog, SEV_INFO, 1, _T("Status:%ul\n"), WinServiceModule::pThis->serviceStatus.dwCurrentState);
	WinServiceModule::pThis->setServiceStatus();

	while (WinServiceModule::pThis->serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		TCHAR buffer[255];
		_stprintf_s(buffer, _T("%u"), i);
		int result = WinServiceModule::pThis->addLogMessage(buffer);
		if (result) {
			WinServiceModule::pThis->serviceStatus.dwCurrentState = SERVICE_STOPPED;
			WinServiceModule::pThis->serviceStatus.dwWin32ExitCode = -1;
			if (WinServiceModule::pThis->serviceStatusHandle!=nullptr)
			{
				SetServiceStatus(WinServiceModule::pThis->serviceStatusHandle, 
					&WinServiceModule::pThis->serviceStatus);
			}
			return;
		}
		i++;
	}
	logPrintf(WinServiceModule::pThis->pLog, SEV_INFO, 1, _T("Status:%ul\n"), WinServiceModule::pThis->serviceStatus.dwCurrentState);
	closeLogFile(WinServiceModule::pThis->pLog);
	return;

}

int WinServiceModule::installService()
{
	if (IsInstalled())
		return EXIT_SUCCESS;
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		STDSTRING sErr = _T("Error: Can't open Service Control Manager");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
		return EXIT_FAILURE;
	}

	hService = CreateService(
		hSCManager,
		(LPCTSTR)szServiceName,
		(LPCTSTR)szServiceName,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		sServicePath.c_str(),
		NULL, NULL, NULL, NULL, NULL
	);

	if (!hService) {
		STDSTRING sErr;
		DWORD err = ::GetLastError();
		switch (err) {
		case ERROR_ACCESS_DENIED:
			sErr = _T("Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			sErr = _T("Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			sErr = _T("Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			sErr = _T("Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			sErr = _T("Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			sErr = _T("Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			sErr = _T("Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			sErr = _T("Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			sErr = _T("Error: ");
			sErr += errTxt.to_string(err);
		}
		addLogMessage(sErr.c_str(), SEV_ERROR);
		CloseServiceHandle(hSCManager);
		return EXIT_FAILURE;
	}
	closeServiceHandle(hService);

	closeServiceHandle(hSCManager);
	addLogMessage(_T("Success install service!"));
	return EXIT_SUCCESS;
}

int WinServiceModule::uninstallService()
{
	if (!IsInstalled())
		return EXIT_SUCCESS;
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager) {
		STDSTRING sErr = _T("Unable to open Service Control Manager. Error: ");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
		return EXIT_FAILURE;
	}
	hService = OpenService(hSCManager, (LPCTSTR)szServiceName, SERVICE_STOP | DELETE);
	if (!hService) {
		STDSTRING sErr = _T("Unable to remove service. Error: ");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
		CloseServiceHandle(hSCManager);
		return EXIT_FAILURE;
	}

	DeleteService(hService);
	closeServiceHandle(hService);
	closeServiceHandle(hSCManager);
	addLogMessage(_T("Success remove service!"));
	return EXIT_SUCCESS;
}

void WinServiceModule::closeServiceHandle(SC_HANDLE& handle)
{
	if (handle)
	{
		CloseServiceHandle(handle);
		ZeroMemory(&handle, sizeof(SC_HANDLE));
	}
}

int WinServiceModule::startService() 
{
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	hService = OpenService(hSCManager, (LPCTSTR)szServiceName, SERVICE_START);
	if (!StartService(hService, 0, NULL)) {
		CloseServiceHandle(hSCManager);
		STDSTRING sErr = _T("Unable to start service. Error: ");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
		return EXIT_FAILURE;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return EXIT_SUCCESS;
}

int WinServiceModule::stopService()
{
	if (hServiceStopEvent == nullptr)
	{
		hServiceStopEvent = OpenEvent(EVENT_MODIFY_STATE, TRUE, szServiceName);
	}
	if (hServiceStopEvent != nullptr)
	{
		SetEvent(hServiceStopEvent);
	}
	hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	hService = OpenService(hSCManager, (LPCTSTR)szServiceName, SERVICE_STOP);
	if (!StartService(hService, 0, NULL)) {
		CloseServiceHandle(hSCManager);
		STDSTRING sErr = _T("Unable to stop service. Error: ");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
		return EXIT_FAILURE;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return EXIT_SUCCESS;
}
extern "C" {
	VOID WINAPI __ServiceMain(DWORD   dwNumServicesArgs, LPTSTR* lpServiceArgVectors)
	{
		WinServiceModule::_ServiceMain(0, NULL);
	}
}
extern "C" {
	int startServiceDispatcher(SERVICE_TABLE_ENTRY* pTable, LOGFILE* pLog);
	int startServiceDispatcher2(LPWSTR lpServiceName, LPSERVICE_MAIN_FUNCTIONW lpServiceProc, LOGFILE* pLog);
};
TCHAR szEmpty[3] = { _T("") };
int  WinServiceModule::runService()
{
	addLogMessage(_T("Attemtping to run service"), SEV_INFO);
	pServiceTable[0].lpServiceName = szServiceName;
	pServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)__ServiceMain;
	// it is important to close service table
	pServiceTable[1].lpServiceName = szEmpty;
	pServiceTable[1].lpServiceProc = nullptr;
	logPrintf(pLog, SEV_INFO, 100,
		_T("Service table prepared(%p,%p)"),
		pServiceTable[1].lpServiceName, pServiceTable[1].lpServiceProc);
	if (startServiceDispatcher2(szServiceName, __ServiceMain, pLog)!=EXIT_SUCCESS) {
		STDSTRING sErr = _T("Unable to start service. Error: ");
		sErr += errTxt.to_string(::GetLastError());
		addLogMessage(sErr.c_str(), SEV_ERROR);
	}
	else
	{
		addLogMessage(_T("Control dispatcher registered"), SEV_INFO);
		serviceStatus.dwWin32ExitCode = S_OK;
		serviceStatus.dwCheckPoint = 0;
		serviceStatus.dwWaitHint = 0;
		setServiceStatus(SERVICE_START_PENDING);
		addLogMessage(_T("Service status set - step into _ServiceMain"), SEV_INFO);
		WinServiceModule::_ServiceMain(0, NULL);
		addLogMessage(_T("Went out _ServiceMain"), SEV_INFO);
	}
	addLogMessage(_T("Service exited"), SEV_ERROR);
	return EXIT_SUCCESS;
}

int  WinServiceModule::initService()
{
	if (hServiceStopEvent == nullptr)
	{
		addLogMessage(_T("Opening service status event"), SEV_INFO);
		hServiceStopEvent = CreateEvent(NULL, FALSE, FALSE, szServiceName);
	}
	logPrintf(pLog, SEV_INFO, 100, _T("Service status event:%p"), hServiceStopEvent);
	return (hServiceStopEvent==nullptr?EXIT_FAILURE:EXIT_SUCCESS);
}

void WinServiceModule::openLog(const TCHAR* logName)
{
	if (!pLog)
	{
		pLog = openLogFile(logName, SEV_DEBUG);
	}
}

bool WinServiceModule::IsInstalled()
{
	bool bResult = false;

	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		SC_HANDLE hService = ::OpenService(hSCM, szServiceName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			bResult = true;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
	}
	return bResult;
}

bool WinServiceModule::setServiceStatus(DWORD dwStatus)
{
	if (serviceStatusHandle)
	{
		if (dwStatus != 0)
			serviceStatus.dwCurrentState = dwStatus;
		return (SetServiceStatus(serviceStatusHandle, &serviceStatus) == TRUE);
	}
	return false;
}