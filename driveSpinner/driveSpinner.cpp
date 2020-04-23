// driveSpinner.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#include "pch.h"
#include "diagData.h"
#include "DriveSpin.h"

int _tmain(int argc, const TCHAR* argv[])
{
    STDSTRING drive;
    bool      bHelp = false;
    int       nRet = 0;
    bool      bElevate = false;
    for (int i = 1; i < argc && !bHelp; i++)
    {
        if (argv[i][0] == _T('-'))
        {
            switch (argv[i][1])
            {
            case _T('d'):
                if (i + 1 < argc)
                {
                    drive.assign(argv[++i]);
                }
                break;
            case _T('A'):
                bElevate = true;
                break;
            case _T('h'):
            default:
                    bHelp = true;
            }
        }
    }
    if (bHelp || drive.empty())
    {
        const TCHAR* ptr = _tcsrchr(argv[0], PATH_SEP);
        if (ptr == NULL)
        {
            ptr = argv[0];
        }
        else
        {
            ptr++;
        }
        STDCERR << ptr << _T(" [-h]-d <drive letter to spin> [-A]") << std::endl;
        STDCERR << _T("-d <drive letter to spin> - which drive to spin") << std::endl;
        STDCERR << _T("-A - try to acquire administration righs and use physical drive access (process will be restarted)") << std::endl;
        nRet = 1;
    }
    if (bElevate && !sys::DriveSpin::IsElevated())
    {
        DWORD dwRet = sys::DriveSpin::runElevated(argc, argv);
        if (dwRet != 0)
        {
            STDCERR << _T("Unable to elevate privileges. Error '")
                << print::diagData::lastErr.to_string(dwRet) << _T("'. Continue with ordinary access.")
                << std::endl;
        }
        else
        {
            nRet = 2;
        }
    }
    if (nRet == 0)
    {   // spin if no help help or elevation performed
        sys::DriveSpin spinner;
        nRet = spinner.spinDrive(drive);
    }
    STDCOUT << print::diagData::timeStamp << _T("Program finished with result:") << nRet << std::endl;
    return nRet;
}
