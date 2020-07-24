#pragma once
#include <wtypes.h>
namespace sys
{
	class DriveSpin
	{
		unsigned long dwDesiredAccess;
		int readFileContents(const STDSTRING& fileName, LARGE_INTEGER* pos = nullptr);
		int scanFolder(const STDSTRING& rootFolder, int level);
		bool getEnvVar(const TCHAR* varName, STDSTRING& val);
	public:
		static unsigned long runElevated(int argc, const TCHAR* argv[]);
		static int IsElevated();
		static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
		static bool bRun;
		DriveSpin();
		~DriveSpin();
		int spinDrive(STDSTRING& drive);
	};

};