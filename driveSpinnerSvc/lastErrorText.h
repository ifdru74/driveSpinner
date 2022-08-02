#pragma once

namespace print
{
	class lastError
	{
		TCHAR* messageBuffer;
		size_t messageSize;
		unsigned long  dwLanguageId;
	public:
		lastError();
		~lastError();
		const TCHAR* to_string(unsigned long dwErrorCode);
	};
};

