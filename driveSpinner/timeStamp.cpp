#include "pch.h"
#include "timeStamp.h"
#include <iomanip>
#include <cstring>

namespace print
{
    const TCHAR timeStamp::cMinus  = _T('-');
    const TCHAR timeStamp::cDblDot = _T(':');
    const TCHAR timeStamp::cZero   = _T('0');
    const TCHAR timeStamp::cSpace  = _T(' ');
    const TCHAR timeStamp::szFieldSep[] = _T(" | ");
    const TCHAR timeStamp::szBadDtm[] = _T("XXXX-XX-XX XX:XX:XX");

	timeStamp::timeStamp() : t(0), lt{ 0 }
	{
	}
	timeStamp::~timeStamp()
	{
		memset(&t, 0, sizeof(std::time_t));
		memset(&lt, 0, sizeof(std::tm));
	}

#ifdef _UNICODE
    std::wostream& operator<<(std::wostream& str, timeStamp& ts)
#else
    std::ostream& operator<<(std::ostream& str, timeStamp& ts)
#endif // _UNICODE
	{
		ts.t = std::time(nullptr);
        if (localtime_s(&ts.lt, &ts.t) == 0)
        {
            str
                << std::setw(4) 
                << ts.lt.tm_year + 1900 << timeStamp::cMinus
                << std::setw(2) << std::setfill(timeStamp::cZero) 
                << ts.lt.tm_mon << timeStamp::cMinus
                << std::setw(2) << std::setfill(timeStamp::cZero) 
                << ts.lt.tm_mday << timeStamp::cSpace
                << std::setw(2) << std::setfill(timeStamp::cZero) 
                << ts.lt.tm_hour << timeStamp::cDblDot
                << std::setw(2) << std::setfill(timeStamp::cZero) 
                << ts.lt.tm_min << timeStamp::cDblDot
                << std::setw(2) << std::setfill(timeStamp::cZero) 
                << ts.lt.tm_sec;
        }
        else
        {
            str << timeStamp::szBadDtm;
        }
        str << timeStamp::szFieldSep;
        return str;
    }
}