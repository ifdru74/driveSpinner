#pragma once
typedef	unsigned long long ULONG64T;
namespace sys
{
	class SysLoad
	{
		ULONG64T _previousTotalTicks;
		ULONG64T _previousIdleTicks;
		MEMORYSTATUSEX memStat;
		ULONG64T CalculateCPULoad(ULONG64T idleTicks, ULONG64T totalTicks);
		ULONG64T FileTimeToInt64(const FILETIME& ft);
	public:
		SysLoad();
		int GetCPULoad();
		int GetMemLoad();
	};
};