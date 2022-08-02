#pragma once
#ifdef __cplusplus
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#ifndef _UNICODE
#define	STDSTRING	std::string
#define	STDCOUT		std::cout
#define	STDCERR		std::cerr
//#define	OSTRSTREAM	std::ostringstream
#define STDIFSTREAM	std::ifstream
#define STDOFSTREAM	std::ofstream
#define	PATH_SEP	'\\'
#define	STDTOSTRING	std::to_string
#else	// _UNICODE
#define	STDSTRING	std::wstring
#define	STDCOUT		std::wcout
#define	STDCERR		std::wcerr
//#define	OSTRSTREAM	std::wostringstream
#define STDIFSTREAM		std::wifstream
#define STDOFSTREAM		std::wofstream
#define	PATH_SEP	_T('\\')
#define	STDTOSTRING	std::to_wstring
#endif	//_UNICODE
#else	// __cplusplus
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif	// __cplusplus
#include <tchar.h>
#define PAGE_SIZE   8192
