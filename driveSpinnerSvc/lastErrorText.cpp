#include "pch.h"
#include "lastErrorText.h"
#include <Windows.h>
#include <WinBase.h>
namespace print
{
    lastError::lastError() : messageBuffer(nullptr), messageSize(0)
    {
        dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);
    }
    lastError::~lastError()
    {
        if (messageBuffer != nullptr)
        {
            //Free the buffer.
            LocalFree(messageBuffer);
            messageBuffer = nullptr;
            messageSize = 0;
        }
    }
	const TCHAR* lastError::to_string(unsigned long dwErrorCode)
	{
        static TCHAR pEmpty[21];
        memset(pEmpty, 0, sizeof(pEmpty));
        if (dwErrorCode == 0)
        {//No error message has been recorded for dwErrorCode == 0
            return pEmpty;
        }
        else
        {
            if (messageBuffer == nullptr)
            {
                if (FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwErrorCode,
                    dwLanguageId,
                    (LPTSTR)&messageBuffer,
                    (DWORD)messageSize,
                    NULL) == 0)
                {   // all the failure - free mem
                    LocalFree(messageBuffer);
                    messageBuffer = nullptr;
                    messageSize = 0;
                }
            }
            if (messageBuffer == nullptr)
            {   // no memory - allocate new
                messageSize = FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwErrorCode,
                    dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&messageBuffer,
                    (DWORD)0,
                    NULL);
            }
            if (messageBuffer == nullptr)
            {
                _stprintf_s(pEmpty, 21, _T("0x%08X"), dwErrorCode);
            }
        }
        return (messageBuffer == nullptr? pEmpty: messageBuffer);
    }
};