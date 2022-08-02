#pragma once
constexpr auto RESET_CONFIG    = 1;
constexpr auto REMOVE_SERVICE  = 2;
constexpr auto INSTALL_SERVICE = 4;
constexpr auto STOP_SERVICE    = 8;
constexpr auto START_SERVICE   = 16;
#include "logC.h"
#include "lastErrorText.h"
class WinServiceModule
{
protected:
	STDSTRING sServicePath;
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	SERVICE_TABLE_ENTRY* pServiceTable;
	//SERVICE_STATUS_HANDLE m_hServiceStatus;
	//SERVICE_STATUS m_status;
	DWORD dwThreadID;
	BOOL m_bService;
	void closeServiceHandle(SC_HANDLE& handle);
	print::lastError errTxt;

public:
	HANDLE	hServiceStopEvent;
	LOGFILE* pLog;
	TCHAR* szServiceName;
	SERVICE_STATUS_HANDLE	serviceStatusHandle;
	static WinServiceModule* pThis;
	SERVICE_STATUS serviceStatus;
	WinServiceModule(LPCTSTR szSvcName, LPCTSTR szSvcPath);
	virtual ~WinServiceModule();
	WinServiceModule(const WinServiceModule& orig) = delete;
	WinServiceModule& operator=(const WinServiceModule& orig) = delete;
	//static void ControlHandler(DWORD request);
	int  addLogMessage(const TCHAR* szMsg, int iSev=SEV_INFO, int iCode=0);
	//static void serviceMain(int argc, TCHAR* argv[]);
	int  installService();
	int  uninstallService();
	int  startService();
	int  stopService();
	int  runService();
	int  initService();
	void openLog(const TCHAR* logName);
	bool IsInstalled();
	bool setServiceStatus(DWORD dwStatus = 0);
	static void WINAPI _ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv);
private:
	static void WINAPI _Handler(DWORD dwOpcode);
};

