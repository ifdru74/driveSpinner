/**********************************************************
*
* Filename............: logC.c
*
* Project.............: Any
* Purpose.............: file log function implementation
* Author..............: Aleksey Shtykov
* Date of creation....: 13.06.2004
* Last modification...: 07.06.2005
*
* Comments............: full file log with rotation
* 
**********************************************************/
//#define	_NO_FUNC_DEFS_
#include "logC.h"
//#include "c_util.h"
#define	DEF_LOG_COUNT	10	// default log file count
#define	DEF_DAY_LEN		(time_t)86400L
#ifdef _UNICODE
#ifndef _UNICODE_LOGS
#define	_CONVERT2MULTIBYTE
#endif // !_UNICODE_LOGS
#endif
int	lastLogError=0;
#if defined(_MSC_VER) || defined(__BORLANDC__)	// fucking M$DO$ text format is still supported in notepad!
//static const TCHAR* recFmt = _T("%04d.%02d.%02d %02d:%02d:%02d | %c%05d | %s\r\n");
static const TCHAR* recFmt = _T("%s%c%05d%s%s\r\n");
static const TCHAR* repFmt = _T("%s%c%05d%slast message repeated %d times\r\n");

static struct tm* localtime_r(const time_t *clock, struct tm *out)
{
    struct tm *ptm;
    errno_t err;
    ptm = NULL;
    if (out != NULL)
    {
        err = localtime_s(out, clock);
        if (err == NOERROR) {
            ptm = out;
        }
    }
    return out;
}

#ifndef _LOG_C_USE_EXTERNAL_TIME_OF_DAY_
static int	gettimeofday(MyTimeval *ptv, struct timezone* nptr)
{
    int	nRet;
    struct timeb tb;
    nRet = -1;
    if (ptv != NULL) {
        ftime(&tb);
        ptv->tv_sec = (long)tb.time;
        ptv->tv_usec = tb.millitm * 1000;
        nRet = 0;
    }
    return nRet;
}
#endif // _LOG_C_USE_EXTERNAL_TIME_OF_DAY_

#else			// the right way gives you a one char per line economy
//static const TCHAR* recFmt = _T("%04d.%02d.%02d %02d:%02d:%02d | %c%05d | %s\n");
static const TCHAR* recFmt = _T("%s%c%05d%s%s\n");
static const TCHAR* repFmt = _T("%s%c%05d%slast message repeated %d times\n");
const TCHAR*  _tcscpy_s(TCHAR* pDst, size_t dstSize, const TCHAR* pSrc)
{
    if(dstSize > _tcslen(pDst))
        return strncpy(pDst, pSrc, dstSize - _tcslen(pDst));
    return NULL;
}

TCHAR*  _tcscat_s(TCHAR* pDst, size_t dstSize, const TCHAR* pSrc)
{
    if(dstSize > _tcslen(pDst))
        return strncat(pDst, pSrc, dstSize - _tcslen(pDst));
    return NULL;
}

TCHAR*  _tcsncat_s(TCHAR* pDst, size_t dstSize, const TCHAR* pSrc, size_t srcCount)
{
    if(dstSize > _tcslen(pDst))
    {
        size_t  nLeft = dstSize - _tcslen(pDst);
        if(nLeft<=srcCount)
            return strncat(pDst, pSrc, nLeft);
        return strncat(pDst, pSrc, srcCount);
    }
    return NULL;
}

#endif
static const TCHAR* defLogDateFmt = _T("%Y.%m.%d %H:%M:%S");
static const TCHAR* defLogFldSep = _T(" | ");

TCHAR*	trim(TCHAR* str)
{
    TCHAR*	ptr;
    if(str!=NULL)	{
        ptr = str + _tcslen(str) - 1;
        if(ptr>str)	{
            while((*ptr==_T('\n')||*ptr==_T('\r')||*ptr==_T('\t')||*ptr==_T(' '))&&ptr>=str)	{
                *ptr = _T('\0');
                ptr --;
            }
        }
    }
    else
        ptr = NULL;
    return ptr;
}

size_t	setLogName(LOGFILE* lf, const TCHAR* name)
{
    size_t	n=0;

    if(lf!=NULL&&name!=NULL)	{
        n = _tcslen(name) + 1;
        if(lf->logName!=NULL)
            free(lf->logName);
        lf->logName = (TCHAR*)calloc(n,sizeof(TCHAR));
        if(lf->logName!=NULL)
            _tcscpy_s(lf->logName, n, name);
        else
            n = 0;
    }
    return n;
}

size_t	setLogFormName(LOGFILE* lf, const TCHAR* name)
{
    size_t	n=0;

    if(lf!=NULL&&name!=NULL)	{
        n = _tcslen(name) + 1;
        if(lf->logFormName!=NULL)
            free(lf->logFormName);
        lf->logFormName = (TCHAR*)calloc(n,sizeof(TCHAR));
        if (lf->logFormName != NULL)
        {
            _tcscpy_s(lf->logFormName, n, name);
        }
        else
            n = 0;
    }
    return n;
}

LOGFILE*	openLogFile(const TCHAR* fileName,int logLevel)
{
    int	nFd;
    LOGFILE*	lf;
    errno_t	err;
    lastLogError = 0;
    lf = NULL;
    int open_flags = O_WRONLY | O_APPEND | O_BINARY;
    if(ACCESS(fileName, W_OK)<0)
    {
       open_flags |= O_CREAT; 
    }
    else
    {
        _tchmod(fileName,S_IREAD | S_IWRITE);
    }
#if defined(_MSC_VER) || defined(__BORLANDC__)
    err = _tsopen_s(&nFd, fileName, open_flags, _SH_DENYNO, S_IREAD | S_IWRITE | S_IRGRP | S_IROTH);
#else
    errno = 0;
    nFd = _topen(fileName, open_flags, S_IREAD | S_IWRITE | S_IRGRP | S_IROTH);
    err = errno;
#endif
    if(err!=NOERROR && nFd<0)	{
        lastLogError = err;
    }
    else	{
        lf = (LOGFILE*)calloc(1,sizeof(LOGFILE));
        if(lf!=NULL)	{
            memset(lf,0,sizeof(LOGFILE));
            _tcscpy_s(lf->logDateFmt, TIME_SIZE, defLogDateFmt);
            _tcscpy_s(lf->logFieldSep, TIME_SIZE, defLogFldSep);
            lf->nFd = nFd;
            lf->logStart = time(NULL);
            if(logLevel>0)
                lf->logLevel = logLevel;
            setLogName(lf,fileName);
            lf->nLogCount = DEF_LOG_COUNT;
            lf->nLogFmt = 4096;
            lf->logFmt = (TCHAR*)calloc(lf->nLogFmt, sizeof(TCHAR));
            if(lf->logFmt!=NULL)
                memset(lf->logFmt, 0, lf->nLogFmt);
            else
                lf->nLogFmt = 0;
            lf->nLogPrt = 4096;
            lf->logPrt = (TCHAR*)calloc(lf->nLogPrt, sizeof(TCHAR));
            if(lf->logPrt!=NULL)
                memset(lf->logPrt, 0, lf->nLogPrt);
            else
                lf->nLogPrt = 0;
            lf->nLRLen = 4096;
            lf->logRec = (TCHAR*)calloc(lf->nLRLen, sizeof(TCHAR));
            lf->logRecP = (TCHAR*)calloc(lf->nLRLen, sizeof(TCHAR));
            if(lf->logRec!=NULL)
                memset(lf->logRec, 0, lf->nLRLen);
            else
                lf->nLRLen = 0;
            if(lf->logRecP!=NULL)
                memset(lf->logRecP, 0, lf->nLRLen);
#ifdef	_REENTRANT
            M_INIT(&lf->mPrint);
            M_INIT(&lf->mRec);
#endif
        }
        else
            lastLogError = E_LOC_MEM;
    }
    return lf;
}

int	freeMem(LOGFILE* lf)
{
    int	nRet=-1;
    if(lf!=NULL)	{
        if(lf->logRec!=NULL)
            free(lf->logRec);
        if(lf->logRecP!=NULL)
            free(lf->logRecP);
        if(lf->logFmt!=NULL)
            free(lf->logFmt);
        if(lf->logPrt!=NULL)
            free(lf->logPrt);
#ifdef	_UNICODE
        if(lf->pLogR!=NULL)
            free(lf->pLogR);
        lf->pLogR = NULL;
#endif
        lf->logFmt = NULL;
        lf->logPrt = NULL;
        lf->logRec = NULL;
        lf->logRecP = NULL;
#ifdef	_REENTRANT
        M_DESTROY(&lf->mPrint);
        M_DESTROY(&lf->mRec);
#endif
        free(lf);
        nRet = 0;
    }
    return nRet;
}

int	closeLogFile(LOGFILE* lf)
{
    int	nRet=-1;

    lastLogError = 0;
    if(lf!=NULL)	{
        if(lf->nFd>0)	{
            nRet = _CLOSE(lf->nFd);
            if(nRet<0)
                lastLogError = errno;
        }
        else
            nRet = E_INV_VAL;
        freeMem(lf);
    }
    else	{
        nRet = E_NUL_PTR;
    }
    return nRet;
}
void	cleanUpDir(LOGFILE* lf)
{
    TCHAR	*ptr, *fileName;
    int	logRecord(LOGFILE* lf, int severity,int recCode,TCHAR* recStr);
#if defined(_MSC_VER) || defined(__BORLANDC__)
    time_t	clTime;
    struct tm lt;
    HANDLE	hFind;
    WIN32_FIND_DATA	wfs;
    SYSTEMTIME	sys_time;

    clTime = time(NULL) - lf->nLogCount * DEF_DAY_LEN;
#endif
    if(lf!=NULL)	{
        if(lf->logName!=NULL)	{
            fileName = (TCHAR*)calloc(_tcslen(lf->logName)+2,sizeof(TCHAR));
            if(fileName!=NULL)	{
                _tcscpy_s(fileName, _tcslen(lf->logName), lf->logName);
                ptr = fileName;
                do	{
                    ptr = _tcschr(ptr,_T('%'));
                    if(ptr!=NULL)	{
                        switch(ptr[1])	{
                        case _T('d'):
                        case _T('m'):
                        case _T('y'):
                            *ptr = _T('?');
                            ptr ++;
                            *ptr = _T('?');
                            ptr ++;
                            break;
                        default:
                            ptr ++;
                        }
                    }
                }	while(ptr!=NULL && (ptr-fileName)<(int)_tcslen(fileName));
#if defined(_MSC_VER) || defined(__BORLANDC__)
                hFind =  FindFirstFile(fileName,&wfs);
                if(hFind!=INVALID_HANDLE_VALUE)	{
                    do	{
                        if(!(wfs.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
                        {
                            if(FileTimeToSystemTime(&wfs.ftCreationTime, &sys_time))
                            {
                                memset(&lt, 0, sizeof(struct tm));
                                lt.tm_mday = sys_time.wDay;
                                lt.tm_mon = sys_time.wMonth - 1;
                                lt.tm_year = sys_time.wYear - 1900;
                                if(mktime(&lt)<clTime)	{
                                    if(!DeleteFile(wfs.cFileName))
                                    {
                                        _WRITE(lf->nFd,wfs.cFileName, (int)_tcslen(wfs.cFileName));
                                    }
                                    else
                                        _WRITE(lf->nFd,wfs.cFileName, (int)_tcslen(wfs.cFileName));
                                    _WRITE(lf->nFd,"\n",1);
                                }
                            }
                        }
                    }	while(FindNextFile(hFind, &wfs));
                    FindClose(hFind);
                }
#else
#endif
                free(fileName);
            }
        }
    }
}
int	logFileOut(LOGFILE* lf, TCHAR* message)
{
	int	nRet = 0;
    char* pRealloc;
#ifdef _CONVERT2MULTIBYTE
	size_t	nConv;
#endif
	if(message!=NULL)	{
#ifdef _CONVERT2MULTIBYTE
            if(_tcslen(lf->logRec)+1>lf->nLogR)	{
                if (lf->pLogR == NULL)
                {
                    if ((lf->pLogR = (char*)malloc(_tcslen(message) + TIME_SIZE + 1)) != NULL)
                    {
                        lf->nLogR = _tcslen(message) + TIME_SIZE + 1;
                    }
                }
                else
                {
                    pRealloc = (char*)realloc(lf->pLogR, _tcslen(message) + TIME_SIZE + 1);
                    if (pRealloc != NULL)
                    {
                        lf->pLogR = pRealloc;
                        lf->nLogR = _tcslen(message) + TIME_SIZE + 1;
                    }
                }
            }
            if(lf->pLogR!=NULL)	{
                memset(lf->pLogR,0,_tcslen(lf->logDate)+1);
                if(wcstombs_s(&nConv, lf->pLogR, lf->nLogR, lf->logDate,_tcslen(lf->logDate))==NOERROR)	{
                    nRet = (int)_WRITE(lf->nFd, lf->pLogR, (int)_tcslen(lf->logDate));
                    memset(lf->pLogR,0,_tcslen(message)+1);
                    if(wcstombs_s(&nConv, lf->pLogR, lf->nLogR, message,_tcslen(message)) == NOERROR)
                        nRet = (int)_WRITE(lf->nFd, lf->pLogR, (int)_tcslen(message));
                    else
                        nRet = E_CONV_UNI;
                }
                else
                    nRet = E_CONV_UNI;
            }
            else	{
                nRet = E_CONV_BUF;
                lf->nLogR = 0;
            }
#else
            nRet = _WRITE(lf->nFd,lf->logDate,(int)_tcslen(lf->logDate));
            nRet = _WRITE(lf->nFd,message, (int)_tcslen(message));
#endif // CONVERT2MULTIBYTE
	}
	return nRet;
}

int	logRecord(LOGFILE* lf, int severity,int recCode,TCHAR* recStr) {
    //time_t	lTime;
    struct tm lt, lt2;
    struct tm *lt1;
    int nRet = 0;
    TCHAR rt;
    TCHAR* pRealloc;
    size_t n;
    int reopenLogInternal(LOGFILE* lf, time_t lTime);
    TCHAR szMsg[TIME_SIZE];
    UTM_BUF tBuf;
    MyTimeval tv;

    if (lf != NULL)
    {
#ifdef _REENTRANT
        M_LOCK(&lf->mRec);
#endif
        //lTime = time(NULL);
        if (gettimeofday(&tv, NULL) < 0)
        {
            memset(&tv, 0, sizeof (MyTimeval));
            tv.tv_sec = (long) time(NULL);
        }
        tBuf.actime = tv.tv_sec; //lTime;
        tBuf.modtime = tv.tv_sec; //lTime;
        //lt1 = localtime_r(&lTime, &lt);
        lt1 = localtime_r(&tBuf.actime, &lt);
        if (lt1 != NULL)
        {
            if (severity <= lf->logLevel)
            {
                switch (severity)
                {
                    case SEV_FATAL:
                        rt = 'F';
                        break;
                    case SEV_ERROR:
                        rt = 'E';
                        break;
                    case SEV_WARN:
                        rt = 'W';
                        break;
                    case SEV_INFO:
                        rt = 'I';
                        break;
                    case SEV_DEBUG:
                        rt = 'D';
                        break;
                    case SEV_EXTRA:
                        rt = 'X';
                        break;
                    default:
                        rt = 'U';
                }
                trim(recStr);
                n = _tcslen(recStr);
                if (n + TIME_SIZE >= lf->nLRLen)
                {
                    if (lf->logRec != NULL)
                        free(lf->logRec);
                    lf->logRec = (TCHAR*) calloc(n + TIME_SIZE, sizeof (TCHAR));
                    if (lf->logRec != NULL)
                        _tcscpy_s(lf->logRec, (n + TIME_SIZE), lf->logRecP);
                    pRealloc = (TCHAR*) realloc(lf->logRecP, (n + TIME_SIZE) * sizeof (TCHAR));//lf->logRecP
                    if (pRealloc != NULL && lf->logRec != NULL)
                    {
                        lf->logRecP = pRealloc;
                        lf->nLRLen = n + TIME_SIZE;
                        _tcscpy_s(lf->logRecP, lf->nLRLen, lf->logRec);
                    }
                }
                if (lf->logRec != NULL)
                {
                    // make date time
                    memset(lf->logDate, 0, TIME_SIZE);
                    _tcsftime(lf->logDate, TIME_SIZE - 1, lf->logDateFmt, &lt);
                    _stprintf_s(szMsg, TIME_SIZE, _T(".%03d"), (int) (tv.tv_usec / 1000));
                    _tcscat_s(lf->logDate, TIME_SIZE, szMsg);
                    if (_tcslen(recStr) > 0 && *recStr != 0)
                    {
                        _stprintf_s(lf->logRec, lf->nLRLen, recFmt,
                            lf->logFieldSep, rt, recCode, lf->logFieldSep, recStr);
                    }
                    else
                    {
                        _stprintf_s(lf->logRec, lf->nLRLen, recFmt,
                            lf->logFieldSep, rt, 0, lf->logFieldSep, _T("<empty>"));
                    }
                    if (_tcscmp(lf->logRec, lf->logRecP) || lf->nLogRepeat < 1) {
                        if (lf->nLogRepeat > 1) {
                            _stprintf_s(szMsg, TIME_SIZE, repFmt,
                                    lf->logFieldSep, rt, 0, lf->logFieldSep, lf->nLogRepeat);
                            nRet = logFileOut(lf, szMsg);
                        }
                        lf->nLogRepeat = 0;
                        nRet = logFileOut(lf, lf->logRec);
                        _tcscpy_s(lf->logRecP, lf->nLRLen, lf->logRec);
                    } 
                    else
                    {
                        _tutime(lf->logName, &tBuf);
                        lf->nLogRepeat++;
                    }
                    if (nRet < 0)
                        lastLogError = errno;
                } 
                else {
                    nRet = E_LOC_MEM;
                    lf->nLRLen = 0;
                }
            }
        } 
        else {
            nRet = E_SYS_MEM;
        }
        lt1 = localtime_r(&lf->logStart, &lt2);
        if (lt.tm_mday != lt2.tm_mday)
            reopenLogInternal(lf, tv.tv_sec);
#ifdef _REENTRANT
        M_UNLOCK(&lf->mRec);
#endif
    } 
    else {
        nRet = E_NUL_PTR;
    }
    return nRet;
}

void	setLevel(LOGFILE* lf,int logLevel)
{
    if(logLevel>=0&&lf!=NULL)
        lf->logLevel = logLevel;
}

int	logDump(LOGFILE* lf,int severity, int recCode, unsigned char *data,int len)
{
    int	nRet=0;
    TCHAR	hex[TIME_SIZE],sym[TIME_SIZE],str[TIME_SIZE];
    unsigned char ch;
    int	i,j;
    if(lf!=NULL)	{
        for(i=0; i<len; i+= 16)	{
            memset(hex,0,TIME_SIZE*sizeof(TCHAR));
            memset(sym,0,TIME_SIZE*sizeof(TCHAR));
            for(j=0; j<16; j++)	{
                if(i+j<len)	{
                    ch = data[i+j];
                    _stprintf_s(str, TIME_SIZE, _T("%02X "),ch);
                    _tcsncat_s((TCHAR*)hex, TIME_SIZE, (TCHAR*)str,3);
                    if(ch>31&&ch<128)
                        sym[j] = ch;
                    else
                        sym[j] = '.';
                }
                else	{
                    _tcscat_s((TCHAR*)hex, TIME_SIZE, _T("   "));
                    _tcscat_s((TCHAR*)sym, TIME_SIZE, _T(" "));
                }
            }
            _tcscat_s((TCHAR*)hex, TIME_SIZE, _T(" "));
            _tcscat_s((TCHAR*)hex, TIME_SIZE, (TCHAR*)sym);
            logRecord(lf,severity,recCode,(TCHAR*)hex);
        }
    }
    else
        nRet = E_NUL_PTR;
    return nRet;
}

LOGFILE*	openLogForm(const TCHAR* logFName,int logLevel)
{
	TCHAR	*fName;
	size_t	nLen;
	time_t	lTime;
	struct tm *lt, lt1;
	LOGFILE	*lf;

	lTime = time(NULL);
	lf = NULL;
	lt = localtime_r(&lTime, &lt1);
	if(lt!=NULL)	{
            nLen = _tcslen(logFName) + TIME_SIZE;
            fName = (TCHAR*)calloc(nLen,sizeof(TCHAR));
            if(fName!=NULL)	{
                memset(fName,0,nLen);
                _tcsftime(fName, nLen-1, logFName, &lt1);
                lf = openLogFile(fName,logLevel);
                if(lf!=NULL)	{
                        setLogFormName(lf,logFName);
                }
                free(fName);
            }
            else	{
                lastLogError = E_LOC_MEM;
            }
	}
	else
            lastLogError = E_SYS_MEM;
	return lf;
}

int	reopenLogInternal(LOGFILE* lf, time_t lTime)
{
	struct tm *lt1, *lt2, lt12, lt22;
	int	nRet=0;
	LOGFILE* lf1;

	if(lf!=NULL)	{
            lt1 = localtime_r(&lTime,&lt12);
            lt2 = localtime_r(&lf->logStart, &lt22);
            if(lt1->tm_mday!=lt2->tm_mday)	{// need to reopen
                cleanUpDir(lf);
                _CLOSE(lf->nFd);
                lf->nFd = -1;
                if(lf->logFormName!=NULL)
                    lf1 = openLogForm(lf->logFormName,lf->logLevel);
                else
                    lf1 = openLogFile(lf->logName,lf->logLevel);
                if(lf1!=NULL)	{
                    lf->nFd = lf1->nFd;
                    lf->logStart  = lf1->logStart;
                    freeMem(lf1);
                }
                lf->nLogRepeat = 0;
            }
	}
	else
            nRet = E_NUL_PTR;
	return nRet;
}

int	reopenLog(LOGFILE* lf, time_t lTime)
{
    int	nRet=0;
	
#ifdef	_REENTRANT
    M_LOCK(&lf->mPrint);
    M_LOCK(&lf->mRec);
#endif
    nRet = reopenLogInternal(lf, lTime);
#ifdef	_REENTRANT
    M_UNLOCK(&lf->mPrint);
    M_UNLOCK(&lf->mRec);
#endif
    return nRet;
}

int	logPrintf(LOGFILE* lf,int severity, int recCode, const TCHAR *format,...)
{
    int	nRet = -3;
    size_t	nLen;
	TCHAR*	pRealloc;
    va_list ap;

    if(format==NULL)
        nRet = -2;
    else
    {
        nLen = _tcslen(format) + _tcslen(recFmt);
        if(lf!=NULL)
        {
#ifdef	_REENTRANT
            M_LOCK(&lf->mPrint);
#endif
#ifndef	_STD_VS_VAR_ARGS_
            va_start( ap );
#else
            va_start(ap, format);
#endif

            if(lf->nLogPrt<nLen)
            {
                pRealloc = (TCHAR*)realloc((void*)lf->logPrt, (nLen+4096)*sizeof(TCHAR));
                if (pRealloc != NULL)
                {
                    lf->logPrt = pRealloc;
                    lf->nLogPrt = nLen + 4096;
                }
            }
            if(lf->logPrt!=NULL)
            {
#if defined(_MSC_VER) || defined(__BORLANDC__)
                nRet = _vstprintf_s(lf->logPrt, lf->nLogPrt, format, ap);
#else
                nRet = vsnprintf(lf->logPrt, lf->nLogPrt, format, ap);
#endif
                if(nRet>=0)
                    logRecord(lf,severity, recCode, lf->logPrt);
            }
            va_end( ap );
#ifdef	_REENTRANT
            M_UNLOCK(&lf->mPrint);
#endif
        }
    }
    return nRet;
}
