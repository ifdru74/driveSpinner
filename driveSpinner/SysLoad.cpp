#include "pch.h"
#include "SysLoad.h"
/*
Resource Links:
Calling memory info in c++:                             http://msdn.microsoft.com/en-us/library/aa366589%28VS.85%29.aspx
I/O file handling in c++:                               http://www.cplusplus.com/doc/tutorial/files/
Date and Time in c++:                                   http://www.tutorialspoint.com/cplusplus/cpp_date_time.htm
CPU Load Percent (Credit to Jeremy Friesner):           https://stackoverflow.com/questions/23143693/retrieving-cpu-load-percent-total-in-windows-with-c
Everything else (too many to list):                     https://stackoverflow.com/
*/

/*
Performance Snapshot Tool

Grabs CPU load percent and basic Memory info from the system,
and or the Windows Task manager

Designed to work with specifically Windows 7 and beyond

Ideology: Create a small executable program to retrieve and
write to a log file a data sample from system performance
in a single snapshot -- robust enough to be called multiple
times per boot

The compiled .exe will be called by another program to run at
an exact, specified time relative to the program that is
calling it

Does 5 checks per second, every 200 milliseconds for a "Snapshot"
of performance

Initial Code Author:    Anonymous
Current Author: Anonymous
Revision:           0.01
Date:               18/4/2014
*/


//included libraries/functionality for input/output
using namespace std;
using namespace sys;

//creates a static variable to convert Bytes to Megabytes
constexpr auto MB = 1048576;
constexpr int  FULL100 = 100;
constexpr ULONG64T  FULL100U = 100;
constexpr ULONG64T  ZEROU = 0;

//functions to calculate and retrieve CPU Load information


ULONG64T SysLoad::CalculateCPULoad(ULONG64T idleTicks, ULONG64T totalTicks)
{
    ULONG64T nRet = ZEROU;
    ULONG64T totalTicksSinceLastTime = totalTicks - _previousTotalTicks;
    ULONG64T idleTicksSinceLastTime  = idleTicks - _previousIdleTicks;

    if (totalTicksSinceLastTime > 0)
    {
        nRet = (idleTicksSinceLastTime * FULL100U) / totalTicksSinceLastTime;
    }

    _previousTotalTicks = totalTicks;
    _previousIdleTicks = idleTicks;
    return FULL100U - nRet;
}

unsigned long long SysLoad::FileTimeToInt64(const FILETIME& ft)
{
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

// Returns 1.0f for "CPU fully pinned", 0.0f for "CPU idle", or somewhere in between
// You'll need to call this at regular intervals, since it measures the load between
// the previous call and the current one.  Returns -1.0 on error.
int SysLoad::GetCPULoad()
{
    FILETIME idleTime, kernelTime, userTime;
    int nRet = 100;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
    {
        return (int)CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime));
    }
    return FULL100;
}

int SysLoad::GetMemLoad()
{
    memStat.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memStat);
    return memStat.dwMemoryLoad;
}

SysLoad::SysLoad() : _previousTotalTicks(ZEROU), _previousIdleTicks(ZEROU)
{
    ::memset(&memStat, 0, sizeof(MEMORYSTATUSEX));
    memStat.dwLength = sizeof(MEMORYSTATUSEX);
}