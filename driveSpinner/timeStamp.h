#pragma once

namespace print
{
	class timeStamp
	{
		std::time_t t;
		std::tm lt;
	public:
		static const TCHAR cMinus;
		static const TCHAR cDblDot;
		static const TCHAR cZero;
		static const TCHAR cSpace;
		static const TCHAR szFieldSep[];
		static const TCHAR szBadDtm[];
		timeStamp();
		~timeStamp();
#ifdef _UNICODE
		friend std::wostream& operator<<(std::wostream& str, timeStamp& ts);
#else
		friend std::ostream& operator<<(std::ostream& str, timeStamp& ts);
#endif // _UNICODE
	};
}