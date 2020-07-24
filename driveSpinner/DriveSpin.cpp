#include <Windows.h>
#include <WinBase.h>
#include "pch.h"
#include "DriveSpin.h"
#include "diagData.h"
#if _MSC_VER>=1900 && __cplusplus>=201703L
#include <filesystem>
#define STD_FILESYSTEM  std::filesystem
#define STD_FSPATH      std::filesystem::path
#define STD_FSDIRITR    std::filesystem::directory_iterator
#else
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
#define STD_FILESYSTEM  std::experimental::filesystem
#define STD_FSPATH      std::experimental::filesystem::path
#define STD_FSDIRITR    std::experimental::filesystem::directory_iterator
#endif
namespace sys
{
    bool DriveSpin::bRun = true;
    unsigned long DriveSpin::runElevated(int argc, const TCHAR* argv[])
    {
        HMODULE hMod = GetModuleHandle(NULL);
        if (hMod != NULL)
        {
            TCHAR szFileName[MAX_PATH*2];
            if (GetModuleFileName(hMod, szFileName, MAX_PATH * 2) != 0)
            {
                static const TCHAR  szSpace[] = _T(" ");
                STDSTRING sCmdParams;
                for (int i = 1; i < argc; i++)
                {
                    if (_tcscmp(argv[i], _T("-A")) != 0)
                    {   // all except elevation
                        sCmdParams.append(szSpace);
                        sCmdParams.append(argv[i]);
                    }
                }
                HINSTANCE hInst = ShellExecute(NULL,
                    _T("runas"),
                    szFileName,
                    sCmdParams.c_str(),
                    NULL,                        // default dir 
                    SW_SHOWNORMAL
                );
                DWORD dwRet = (DWORD)hInst;
                switch (dwRet)
                {
                case 0:
                case SE_ERR_OOM:
                    dwRet = ERROR_OUTOFMEMORY;
                    break;
                case ERROR_FILE_NOT_FOUND:// also SE_ERR_FNF
                case ERROR_PATH_NOT_FOUND:// also SE_ERR_PNF
                case ERROR_BAD_FORMAT:
                    //dwRet = (DWORD)hInst;
                    break;
                case SE_ERR_ACCESSDENIED:
                    dwRet = ERROR_ACCESS_DENIED;
                    break;
                case SE_ERR_SHARE:
                    dwRet = ERROR_SHARING_VIOLATION;
                    break;
                case SE_ERR_DLLNOTFOUND:
                    dwRet = ERROR_MOD_NOT_FOUND;
                    break;
                case SE_ERR_DDETIMEOUT:
                    dwRet = ERROR_TIMEOUT;
                    break;
                case SE_ERR_DDEFAIL:
                    dwRet = ERROR_FATAL_APP_EXIT;
                    break;
                case SE_ERR_DDEBUSY:
                    dwRet = ERROR_BUSY;
                    break;
                case SE_ERR_ASSOCINCOMPLETE:
                    dwRet = ERROR_INVALID_ACCESS;
                default:
                    dwRet = 0;
                    break;
                }
                return (unsigned long)dwRet;
            }
        }
        return (unsigned long)GetLastError();
	}

    int DriveSpin::IsElevated()
    {
        BOOL fRet = FALSE;
        DWORD dwRet = 0;
        HANDLE hToken = NULL;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) 
        {
            TOKEN_ELEVATION Elevation;
            DWORD cbSize = sizeof(TOKEN_ELEVATION);
            if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
                fRet = Elevation.TokenIsElevated;
            }
            else
            {
                dwRet = GetLastError();
                STDCERR << print::diagData::timeStamp 
                    << _T("Unable to GetTokenInformation. Error:") 
                    << print::diagData::lastErr.to_string(dwRet) << std::endl;
            }
        }
        else
        {
            dwRet = GetLastError();
            STDCERR << print::diagData::timeStamp
                << _T("Unable to OpenProcessToken. Error:")
                << print::diagData::lastErr.to_string(dwRet) << std::endl;
        }
        if (hToken)
        {
            CloseHandle(hToken);
        }
        return fRet;
    }

    BOOL WINAPI DriveSpin::CtrlHandler(DWORD fdwCtrlType)
    {
        BOOL bRet = TRUE;
        switch (fdwCtrlType)
        {
            // Handle the CTRL-C signal. 
        case CTRL_C_EVENT:
            STDCERR << print::diagData::timeStamp << _T("Ctrl-C event") << std::endl;
            Beep(750, 300);
            bRet = TRUE;
            break;

            // CTRL-CLOSE: confirm that the user wants to exit. 
        case CTRL_CLOSE_EVENT:
            Beep(600, 200);
            STDCERR << print::diagData::timeStamp << _T("Ctrl-Close event") << std::endl;
            bRet = TRUE;
            break;

            // Pass other signals to the next handler. 
        case CTRL_BREAK_EVENT:
            Beep(900, 200);
            STDCERR << print::diagData::timeStamp << _T("Ctrl-Break event") << std::endl;
            bRet = TRUE;
            break;

        case CTRL_LOGOFF_EVENT:
            Beep(1000, 200);
            STDCERR << print::diagData::timeStamp << _T("Ctrl-Logoff event") << std::endl;
            bRet = FALSE;
            break;

        case CTRL_SHUTDOWN_EVENT:
            Beep(750, 500);
            STDCERR << print::diagData::timeStamp << _T("Ctrl-Shutdown event") << std::endl;
            bRet = FALSE;
            break;

        default:
            STDCERR << print::diagData::timeStamp << _T("Unknown event") << std::endl;
            bRet = FALSE;
            break;
        }
        DriveSpin::bRun = false;
        return bRet;
    }
    DriveSpin::DriveSpin() : dwDesiredAccess(FILE_READ_ACCESS)
    {
        if (SetConsoleCtrlHandler(DriveSpin::CtrlHandler, TRUE))
        {
            STDCOUT << print::diagData::timeStamp << _T("Control handler installed successfully") << std::endl;
        }
        else
        {
            STDCERR << print::diagData::timeStamp << _T("Control handler was not installed") << std::endl;
        }
    }
    DriveSpin::~DriveSpin()
    {

    }
    int DriveSpin::readFileContents(const STDSTRING& fileName, LARGE_INTEGER* pos)
    {
        int nRet = 0;
        HANDLE hFile = INVALID_HANDLE_VALUE;
        hFile = CreateFile(fileName.c_str(),
            dwDesiredAccess,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING,
            NULL);
        if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
        {
            DWORD  bufLen = PAGE_SIZE;
            char    buf[PAGE_SIZE];
            DWORD   dwRead = 0;
            size_t  currentPos = 0;
            STDCOUT << print::diagData::timeStamp << _T("File '") << fileName << _T("' opened") << std::endl;
            if (pos != nullptr)
            {
                LARGE_INTEGER newPtr;
                if (::SetFilePointerEx(hFile, *pos, &newPtr, SEEK_SET))
                {
                    STDCOUT << print::diagData::timeStamp << _T("File '") << fileName << _T("' new position set to ") << newPtr.QuadPart << std::endl;
                    currentPos = newPtr.QuadPart;
                }
                else
                {
                    STDCOUT << print::diagData::timeStamp << _T("File '") << fileName << _T("' new position was NOT set to ") << pos->QuadPart << std::endl;
                }
            }
            BOOL    bRead = FALSE;
            while (bRun && (bRead = ReadFile(hFile, buf, bufLen, &dwRead, NULL)))
            {
                currentPos += dwRead;
                if (dwRead != bufLen)
                {
                    STDCOUT << print::diagData::timeStamp << _T("End of at position:") << currentPos << std::endl;
                    bRead = TRUE;
                    break;
                }
                else
                {
                    STDCOUT << print::diagData::timeStamp << _T("Read at position:") << currentPos << std::endl;
                }
                ::Sleep(5000);
            }
            if (!bRead)
            {
                DWORD dwLastErr = ::GetLastError();
                STDCERR << print::diagData::timeStamp << _T("Error '") << print::diagData::lastErr.to_string(dwLastErr)
                    << _T("' at position:") << currentPos << std::endl;
            }
            if (!bRun)
            {
                LARGE_INTEGER newPtr;
                memset(&newPtr, 0, sizeof(LARGE_INTEGER));
                if (::SetFilePointerEx(hFile, newPtr, pos, SEEK_CUR))
                {
                    STDCOUT << print::diagData::timeStamp << _T("Stop at position:") << pos->QuadPart << std::endl;
                }
                else
                {
                    STDCOUT << print::diagData::timeStamp << _T("Stop at position:") << currentPos << std::endl;
                }
                nRet = 0;
            }
            else
            {
                memset(pos, 0, sizeof(LARGE_INTEGER));
            }
            CloseHandle(hFile);
            ::Sleep(1000);
        }
        else
        {
            DWORD dwLastErr = ::GetLastError();
            STDCERR << print::diagData::timeStamp << _T("Error '") << print::diagData::lastErr.to_string(dwLastErr) << ("' opening file '") << fileName << _T("'") << std::endl;
            nRet = 2;
        }
        return nRet;
    }

    int DriveSpin::scanFolder(const STDSTRING& rootFolder, int level)
    {
        int nRet = 0;
        STD_FSPATH folderPath(rootFolder);
        if (exists(folderPath))
        {
            STD_FSDIRITR end_itr;
            for (STD_FSDIRITR dirIte(rootFolder); dirIte != end_itr && bRun; ++dirIte)
            {
                STD_FSPATH filePath(dirIte->path());
                try
                {
                    const STDSTRING& rFileName =
#ifdef _UNICODE
                        filePath.wstring();
#else
                        filePath.string();
#endif
                    if (!STD_FILESYSTEM::is_directory(dirIte->status()))
                    {
                        nRet = readFileContents(rFileName, nullptr);
                    }
                    else
                    {
                        nRet = scanFolder(rFileName, level + 1);
                    }
                }
                catch (std::exception& e)
                {
                    STDCERR << print::diagData::timeStamp << _T("Exception '") << e.what()
                        << _T("in scanFolder(") << rootFolder << _T(",")
                        << level << _T(")") << std::endl;
                    nRet = -1;
                }
            }
        }
        return nRet;
    }

    int DriveSpin::spinDrive(STDSTRING& drive)
    {
        int nRet = 0;
        const TCHAR cPathSep = _T('\\');
        if (DriveSpin::IsElevated())
        {   // privileged mode - physical access
            dwDesiredAccess = 0x80000000 | 0x40000000;
            STDCOUT << print::diagData::timeStamp << _T("Privileged mode - direct disc access") << std::endl;
            STDSTRING fileName(_T("\\\\.\\") + drive);
            if (drive.at(drive.length() - 1) != print::timeStamp::cDblDot)
            {
                fileName += print::timeStamp::cDblDot;
            }
            LARGE_INTEGER pos;
            memset(&pos, 0, sizeof(LARGE_INTEGER));
            STDSTRING   cfgFileName;
            if (!getEnvVar(_T("APPDATA"), cfgFileName))
            {
                cfgFileName.assign(drive);
                if (cfgFileName.at(drive.length() - 1) != print::timeStamp::cDblDot)
                {
                    cfgFileName += print::timeStamp::cDblDot;
                }
            }
            cfgFileName += cPathSep;
            cfgFileName += _T("driveSpin");
            ::CreateDirectory(cfgFileName.c_str(), NULL);
            cfgFileName += cPathSep;
            cfgFileName += _T("current.pos");
            STDIFSTREAM cfg;
            cfg.open(cfgFileName.c_str());
            if (cfg.is_open())
            {
                cfg >> pos.QuadPart;
            }
            cfg.close();
            while (bRun && nRet == 0)
            {
                nRet = readFileContents(fileName, &pos);
            }
            STDOFSTREAM ocfg;
            ocfg.open(cfgFileName.c_str());
            if (ocfg.is_open())
            {
                ocfg << pos.QuadPart;
            }
            ocfg.close();
        }
        else
        {   // ordinary mode - read all files
            STDCOUT << print::diagData::timeStamp << _T("Ordinary mode - scan filesystem") << std::endl;
            if (drive.find(print::timeStamp::cDblDot) == std::string::npos)
            {
                drive += print::timeStamp::cDblDot;
            }
            if (drive.find(PATH_SEP) == std::string::npos)
            {
                drive += PATH_SEP;
            }
            nRet = scanFolder(drive, 0);
        }
        return nRet;
    }
    bool DriveSpin::getEnvVar(const TCHAR* varName, STDSTRING& val)
    {
        if (varName != NULL)
        {
            TCHAR buf[4096];
            if (::GetEnvironmentVariable(varName, buf, sizeof(buf)))
            {
                val.assign(buf);
                return true;
            }
        }
        return false;
    }
};