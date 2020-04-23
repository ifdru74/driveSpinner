#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <tchar.h>
#ifndef _UNICODE
#define	STDSTRING	std::string
#define	STDCOUT		std::cout
#define	STDCERR		std::cerr
//#define	OSTRSTREAM	std::ostringstream
#define	PATH_SEP	'\\'
#define	STDTOSTRING	std::to_string
#else
#define	STDSTRING	std::wstring
#define	STDCOUT		std::wcout
#define	STDCERR		std::wcerr
//#define	OSTRSTREAM	std::wostringstream
#define	PATH_SEP	_T('\\')
#define	STDTOSTRING	std::to_wstring
#endif
#define PAGE_SIZE   8192
