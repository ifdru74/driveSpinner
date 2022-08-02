#include "pch.h"
#include <Windows.h>
#include <WinBase.h>
#include <winsvc.h>
#include "logC.h"

int startServiceDispatcher(SERVICE_TABLE_ENTRY* pTable, LOGFILE* pLog)
{
	int nRet = EXIT_FAILURE;
	SERVICE_TABLE_ENTRY pSvc[] = { {pTable[0].lpServiceName, pTable[0].lpServiceProc}, };
	__try
	{
		logPrintf(pLog, SEV_INFO, 101, 
			_T("Before StartServiceCtrlDispatcher(%p,%p)"),
			pSvc[0].lpServiceName, pSvc[0].lpServiceProc);
		if (StartServiceCtrlDispatcher(pSvc))
			nRet = EXIT_SUCCESS;
		logPrintf(pLog, SEV_INFO, 101, _T("After StartServiceCtrlDispatcher"));
	}
	__except(TRUE)
	{
		logPrintf(pLog, SEV_FATAL, 101, 
			_T("Fatal exception %ul while calling StartServiceCtrlDispatcher\n"), 
			GetExceptionCode());
	}
	return nRet;
}

int startServiceDispatcher2(LPWSTR lpServiceName, LPSERVICE_MAIN_FUNCTIONW lpServiceProc, LOGFILE* pLog)
{
	SERVICE_TABLE_ENTRY pSvc[] = { {lpServiceName, lpServiceProc}, };
	return startServiceDispatcher(pSvc, pLog);
}