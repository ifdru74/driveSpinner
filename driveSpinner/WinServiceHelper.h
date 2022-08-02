#pragma once

class WinServiceHelper
{
	TCHAR* szServiceName;
	STDSTRING servicePath;
	HMODULE	hMod;
	SC_HANDLE hSCManager;
	SC_HANDLE hService;
	TCHAR*    szBuf;
protected:
	bool fillModuleInfo(LPCTSTR szModName = NULL);
public:
	WinServiceHelper(const STDSTRING& serviceName);
	~WinServiceHelper();
	bool install();
	bool remove();
	bool start();
	bool stop();
	inline LPTSTR getName() { return szServiceName; };
	SERVICE_STATUS ServiceStatus;
	SERVICE_STATUS_HANDLE hStatus;
	LPHANDLER_FUNCTION	controlHandler;
	bool initServiceStatus(LPHANDLER_FUNCTION handler=NULL);
	void setStatusAndCode(DWORD dsStatus, DWORD dwCode = 0);
	const DWORD  getStatus() const;
};

