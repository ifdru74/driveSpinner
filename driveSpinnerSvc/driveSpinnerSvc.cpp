// driveSpinnerSvc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <Windows.h>
#include "WinServiceModule.h"
#include <filesystem>
#include <cstdio>
TCHAR szServiceName[] = _T("DriveSpinnerSvc");
typedef struct _cmdLineOpt {
	STDSTRING longName;
	STDSTRING shortName;
	int	bits;
}CmdLineOpt;
CmdLineOpt	options[]{
	{_T("--install"),      _T("-i"), INSTALL_SERVICE},
	{_T("--deinstall"),    _T("-d"), REMOVE_SERVICE},
	{_T("--start"),        _T("-s"), START_SERVICE},
	{_T("--stop"),         _T("-t"), STOP_SERVICE},
	{_T("--reset_config"), _T("-R"), RESET_CONFIG},
	{_T("--restart"),      _T("-r"), STOP_SERVICE|START_SERVICE},
	{_T(""), _T(""), 0},
};
int _tmain(int argc, TCHAR* argv[])
{
	int nRet = EXIT_SUCCESS;
	TCHAR szServicePath[MAX_PATH];

	if (!GetModuleFileName(NULL, szServicePath, MAX_PATH))
	{
		_tcsncpy_s(szServicePath, MAX_PATH, argv[0], _tcslen(argv[0]));
	}
	try
	{
		//if (STDCOUT.good())
		//{
		//	STDCOUT << _T("FileName:") << szServicePath << std::endl;
		//}
		WinServiceModule svc(szServiceName, szServicePath);
		if (argc < 2)
		{
			STDSTRING sLogPath = szServicePath;
			size_t pos = sLogPath.find(_T(".exe"));
			if (pos != std::string::npos)
			{
				sLogPath.replace(pos, 4, _T(".log"));
				FILE* fOut;
				FILE* fErr;
				svc.openLog(sLogPath.c_str());
				_wfreopen_s(&fOut, sLogPath.c_str(), _T("w"), stdout);
				_wfreopen_s(&fErr, sLogPath.c_str(), _T("w"), stderr);
			}
			nRet = svc.runService();
		}
		else
		{
			int nActions = 0;
			int i;
			for (i = 1; i < argc; ++i)
			{
				CmdLineOpt* pOpt = options;
				while (pOpt->bits != 0)
				{
					if (pOpt->longName.compare(argv[i]) == 0 || pOpt->shortName.compare(argv[i]) == 0)
					{
						nActions |= pOpt->bits;
						break;
					}
					pOpt++;
				}
			}
			int nLimit = RESET_CONFIG | REMOVE_SERVICE | INSTALL_SERVICE | STOP_SERVICE | START_SERVICE;
			for(i=1; i<=nLimit && nRet==EXIT_SUCCESS; i*=2)
			{
				switch (i & nActions)
				{
				case RESET_CONFIG:
					break;
				case REMOVE_SERVICE:
					if ((nActions & INSTALL_SERVICE) == 0)
					{
						nActions = REMOVE_SERVICE;
					}
					nRet = svc.uninstallService();
					break;
				case INSTALL_SERVICE:
					nRet = svc.installService();
					break;
				case STOP_SERVICE:
					nRet = svc.stopService();
					break;
				case START_SERVICE:
					nRet = svc.startService();
					break;
				case 0:
					break;
				default:
					nRet = EXIT_FAILURE;
				}
			}
		}
	}
	catch (const std::bad_alloc& ba) {
		std::cerr << "Fatal Memory allocation error:" << ba.what() << std::endl;
		nRet = EXIT_FAILURE;
	}
	return nRet;
}
