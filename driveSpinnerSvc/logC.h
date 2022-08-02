#ifndef	_LOGC_H_
#define	_LOGC_H_
/**********************************************************
*
* Filename............: logC.h
*
* Project.............: Any
* Purpose.............: file log function definitions
* Author..............: Aleksey Shtykov
* Date of creation....: 13.06.2004
* Last modification...: 07.06.2005
*
* Comments............: full file log with rotation
* 
**********************************************************/
// common includes
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include "tchar_port.h"


#if defined(_MSC_VER) || defined(__BORLANDC__)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#ifdef	_UNICODE
// for sure
#ifndef	UNICODE
#define	UNICODE
#endif
#endif
#include <windows.h>
#include <winbase.h>
//#include <tchar.h>
#include <io.h>
#include <sys/utime.h>
#include <sys/timeb.h>	// for more than one second quality
typedef	struct _utimbuf UTM_BUF;

// I don't need to include sockets
#ifndef _LOG_C_USE_EXTERNAL_TIME_OF_DAY_
#ifndef _MY_TIMEVAL_
#define	_MY_TIMEVAL_
typedef	long suseconds_t;
typedef struct _mytimeval {
	time_t tv_sec;
	suseconds_t	tv_usec;
} MyTimeval;
#endif // !_MY_TIMEVAL_
#ifndef _MY_TIMEZONE_
#define	_MY_TIMEZONE_
struct timezone
{
	int	dummy;
};
#endif // !_MY_TIMEZONE
#endif	//_LOG_C_USE_EXTERNAL_TIME_OF_DAY_

#ifndef _CLOSE
#define	_CLOSE	_close
#endif // !_CLOSE
#ifndef _WRITE
#define	_WRITE	_write
#endif // !_WRITE
#ifndef W_OK
#define	W_OK	2
#endif // !W_OK
#ifndef ACCESS
#ifdef	_UNICODE
#define	ACCESS	_waccess
#else
#define	ACCESS	_access
#endif	// _UNICODE
#endif // !ACCESS

#else	//defined(_MSC_VER) || defined(__BORLANDC__)

// Win32 must die!
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
typedef	struct utimbuf UTM_BUF;
typedef	struct timeval	ITIMEVAL;
// M$DO$ must DIE!
#ifndef O_BINARY
#define	O_BINARY	0
#endif
// some useless defines from M$ TCHAR.H
#ifndef _CLOSE
#define	_CLOSE	close
#endif // !_CLOSE

#ifndef _WRITE
#define	_WRITE	write
#endif // !_WRITE
#ifndef ACCESS
#define	ACCESS	access
#endif // !ACCESS

#ifndef errno_t
#define errno_t int
#endif

#ifndef _SH_DENYWR
#define _SH_DENYWR  0
#endif

#ifndef	NOERROR
#define	NOERROR   0
#endif

#endif	//defined(_MSC_VER) || defined(__BORLANDC__)

#include <stdarg.h>
#define	_STD_VS_VAR_ARGS_

/* if we have threads we have to pay attention to thread safety */
#ifdef	_REENTRANT
#include "mutex_port.h"
#endif	//_REENTRANT

// fancy file access modes
#ifndef	S_IRGRP
#define	S_IRGRP	0
#endif
#ifndef	S_IROTH
#define	S_IROTH	0
#endif
#ifndef S_IREAD
#define S_IREAD S_IRUSR|S_IRGRP|S_IROTH
#endif
#ifndef S_IWRITE
#define S_IWRITE S_IWUSR
#endif

/* memory sizes defines */
#define	TIME_SIZE	80

/* error codes defines */
#define	E_FILE_OP	-1
#define	E_SYS_MEM	-2
#define	E_LOC_MEM	-3
#define	E_NUL_PTR	-4
#define	E_INV_VAL	-5
#define	E_CONV_UNI	-6
#define	E_CONV_BUF	-7

/* severity code defines */
#define	SEV_FATAL	0	// fatal error
#define	SEV_ERROR	1	// just error
#define	SEV_WARN	2	// warning message
#define	SEV_INFO	3	// normal message
#define	SEV_DEBUG	4	// enormous message
#define	SEV_EXTRA	5	// extra debug message

#ifdef __cplusplus
extern "C" {
#endif

/* log file handler structure */
typedef	struct	logfile	{
	int	nFd;	// file descriptor
	int	logLevel;	// log severity level. severity below this not logged
	size_t	nLRLen;
	size_t	nLogRepeat;	// log repeat count
	TCHAR	logDate[TIME_SIZE]; // log date
	TCHAR	logDateFmt[TIME_SIZE]; // log date format
	TCHAR	logFieldSep[TIME_SIZE]; // log field separator
	TCHAR	*logRec;	// log record
	TCHAR	*logRecP;	// log previous record to prevent repeat rush
	TCHAR	*logName;	// log file name
	TCHAR	*logFormName;	// log file name for openLogForm()
	time_t	logStart;	// log start time
	size_t	nLogCount;	// log count
	TCHAR	*logFmt;	// log format handler for logPrintf()
	size_t	nLogFmt;	//	log format len
	TCHAR	*logPrt;	// log string for logPrintf()
	size_t	nLogPrt;	// log string len
#ifdef	_REENTRANT
	MUTEX	mPrint;	// logPrintf
	MUTEX	mRec;	// logRecord
#endif
#ifdef	_UNICODE
	char*	pLogR;	// buffer for unicode 2 ansi encoding
	size_t	nLogR;	// buffer len
#endif
}LOGFILE;

#ifndef	_NO_FUNC_DEFS_
extern	int	lastLogError;

LOGFILE*	openLogFile(const TCHAR* fileName,int logLevel);
LOGFILE*	openLogForm(const TCHAR* logFName,int logLevel);
int	closeLogFile(LOGFILE* lf);
int	logRecord(LOGFILE* lf, int severity,int recCode,TCHAR* recStr);
int	logDump(LOGFILE* lf,int severity, int recCode, unsigned char *data,int len);
void	setLevel(LOGFILE* lf,int logLevel);
int	logPrintf(LOGFILE* lf,int severity, int recCode, const TCHAR *format, ...);
#endif	/*_NO_FUNC_DEFS_*/

#ifdef __cplusplus
}
#endif
#endif	/*_LOGC_H_*/
