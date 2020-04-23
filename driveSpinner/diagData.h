#pragma once
#include "timeStamp.h"
#include "lastErrorText.h"
namespace print
{

	class diagData
	{
	public:
		static print::timeStamp timeStamp;
		static print::lastError lastErr;
	};
};
