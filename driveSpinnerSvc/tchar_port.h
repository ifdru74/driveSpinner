#pragma once
#ifndef	_TCHAR_PORT_H_
#define	_TCHAR_PORT_H_
#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <tchar.h>
#else
#define	_TCHAR_DEFINED
#define __TCHAR_DEFINED
#ifdef	_UNICODE
// for sure
#ifndef	UNICODE
#define	UNICODE
#endif
#include <wchar.h>
/* typedefs */
typedef	wchar_t*	LPTSTR
typedef	const wchar_t*	LPCTSTR
typedef	wchar_t	TCHAR
/* function map */
#define	_tcscat	wcscat
#define	_tcscmp	wcscmp
#define	_tcscpy	wcscpy
#define	_tcschr	wcschr
#define	_tcsftime	wcsftime
#define	_tcslen	wcslen
#define	_tcsrchr	wcsrchr
#define	_ttoi	wcstol
#define	_ttol	wcstol
#define	_tutime	wutime
#define __T(x)      L ## x
#else
/* typedefs */
typedef	char*	LPTSTR;
typedef	const char*	LPCTSTR;
typedef	char	TCHAR;
/* function map */
#define	_tcscat	strcat
#define	_tcschr	strchr
#define	_tcscmp	strcmp
#define	_tcscpy	strcpy
#define	_tcsstr	strstr
#define _ftprintf   fprintf
#define _stprintf_s snprintf
#define	_tcsftime	strftime
#define	_tcslen	strlen
#define	_tcsrchr	strrchr
#define	_ttoi	atoi
#define	_ttol	atol
#define	_ttof	atof
#define	_tutime	utime
#define	_topen	open
#define	_tchmod	chmod
#define	_stprintf	sprintf
#define	_tcsncat	strncat
#define __T(x)      x
#endif	// unicode
#define _T(x)       __T(x)
#define _TEXT(x)    __T(x)
#define _tmain  main
#define _tcstol strtol
#define	_tfopen	fopen
#define	_tfreopen	freopen
#define	_sntprintf	snprintf
#endif	// win32
#define NULLCH  _T('\0')
#endif	// tchar
