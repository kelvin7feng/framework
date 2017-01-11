//
//  kmacros.h
//  thread
//
//  Created by 冯文斌 on 16/12/7.
//  Copyright © 2016年 kelvin. All rights reserved.
//
#ifndef macros_h
#define macros_h


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <stddef.h>
#include <unistd.h>
#endif


#if defined(CHECK_MEMERY_LEAK) & defined(WIN32) & (defined(DEBUG) | defined(_DEBUG))
#include <crtdbg.h>
#include "vld.h"
#endif


enum KGLOG_PRIORITY
{
    KGLOG_RESERVE0  =   0,  // KGLOG_EMERG  =   0,  // system is unusable
    KGLOG_RESERVE1  =   1,  // KGLOG_ALERT  =   1,  // action must be taken immediately
    KGLOG_RESERVE2  =   2,  // KGLOG_CRIT   =   2,  // critical conditions
    KGLOG_ERR		=   3,  // error conditions
    KGLOG_WARNING	=   4,  // warning conditions
    KGLOG_RESERVE3  =   5,  // KGLOG_NOTICE =   5,  // normal but significant condition
    KGLOG_INFO	    =   6,  // informational
    KGLOG_DEBUG	    =   7,  // debug-level messages
    KGLOG_PRIORITY_MAX
};
#ifdef __cplusplus
extern "C"
{
#endif
    int KGLogPrintf(int Priority, const char cszFormat[], ...);
#ifdef __cplusplus
};
#endif


#ifndef _ASSERT
#define _ASSERT(f)	 if (!(f)) fprintf(stderr, "Assert failed in %s, line = %i\n", __FILE__, __LINE__)
#endif

#define KG_FUNCTION __FUNCTION__

#define KG_USE_ARGUMENT(arg) (arg)

#ifndef KG_PROCESS_ERROR
#define KG_PROCESS_ERROR(Condition) \
do  \
{   \
if (!(Condition))   \
goto Exit0;     \
} while (0)
#endif


#define KG_PROCESS_SUCCESS(Condition) \
do  \
{   \
if (Condition)      \
goto Exit1;     \
} while (false)

#ifndef KGLOG_PROCESS_ERROR
#define KGLOG_PROCESS_ERROR(Condition) \
do  \
{   \
if (!(Condition))       \
{                       \
KGLogPrintf(        \
KGLOG_DEBUG,    \
"KGLOG_PROCESS_ERROR(%s) at line %d in %s\n", #Condition, __LINE__, KG_FUNCTION  \
);				\
goto Exit0;         \
}                       \
} while (0)
#endif

#define KG_ERROR(stream) \
KGLogPrintf(        \
KGLOG_ERR,    \
"[KG_ERROR] %s(%d) %s\n", KG_FUNCTION, __LINE__, (stream)  \
);				\


#define KGLOG_CONFIRM_BREAK(Condition) if (!(Condition)) { KG_ERROR(""); break; }
#define KGLOG_CONFIRM_CONTINUE(Condition) if (!(Condition)) { KG_ERROR(""); continue; }
#define KGLOG_CONFIRM_RET_NULL(Condition) if (!(Condition)) { KG_ERROR(""); return 0; }

#define KGLOG_OUTPUT_ERROR(Condition) \
do  \
{   \
if (!(Condition))       \
{                       \
KGLogPrintf(        \
KGLOG_DEBUG,    \
"KGLOG_PROCESS_ERROR(%s) at line %d in %s\n", #Condition, __LINE__, KG_FUNCTION  \
);                  \
}                       \
} while (false)

#define KGLOG_PROCESS_SUCCESS(Condition) \
do  \
{   \
if (Condition)          \
{                       \
KGLogPrintf(        \
KGLOG_DEBUG,    \
"KGLOG_PROCESS_SUCCESS(%s) at line %d in %s\n", #Condition, __LINE__, KG_FUNCTION  \
);                  \
goto Exit1;         \
}                       \
} while (false)

#define KGLOG_PROCESS_ERROR_RET_CODE(Condition, Code) \
do  \
{   \
if (!(Condition))       \
{                       \
KGLogPrintf(        \
KGLOG_DEBUG,    \
"KGLOG_PROCESS_ERROR_RET_CODE(%s, %d) at line %d in %s\n", \
#Condition, (Code), __LINE__, KG_FUNCTION                  \
);                  \
nResult = (Code);   \
goto Exit0;         \
}                       \
} while (false)


#define KG_COM_PROCESS_ERROR(Condition) \
do  \
{   \
if (FAILED(Condition))  \
goto Exit0;         \
} while (false)


#define KG_COM_PROCESS_SUCCESS(Condition)   \
do  \
{   \
if (SUCCEEDED(Condition))   \
goto Exit1;             \
} while (false)

#define KG_COM_PROC_ERROR_RET_CODE(Condition, Code)     \
do  \
{   \
if (FAILED(Condition))  \
{                       \
hrResult = Code;    \
goto Exit0;         \
}                       \
} while (false)


#define KG_COM_RELEASE(pInterface) \
do  \
{   \
if (pInterface)                 \
{                               \
(pInterface)->Release();    \
(pInterface) = NULL;        \
}                               \
} while (false)

#define  KGLOG_CHECK_RET_REASON(Condition, reason, Reason)	\
if (!(Condition))					\
{								\
reason = Reason;			\
KGLOG_PROCESS_ERROR(false);	\
}

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_FREE
#define SAFE_FREE(p)       { if (p) { free(p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)       { if (p) { delete [] (p);     (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef KG_LOG_WARNING
#define  KG_LOG_WARNING(s) fprintf(stderr, "warning in %s, line = %i content=%s\n", __FILE__, __LINE__,s)
#endif

#ifdef WIN32
#ifndef snprintf
#define snprintf _snprintf
#endif
#endif

#if !defined(WIN32)
#define KSLEEP(X)  usleep((X) * 1000)
#else
#define KSLEEP(X)  ::Sleep(X)
#endif

#define KLUA_SAFECALL_BEGIN(l)	\
int _nLuaSafeTopBegin = lua_gettop((l));\
int _nLuaSafeTopEnd = _nLuaSafeTopBegin;

#define KLUA_SAFECALL_END(l)	\
_nLuaSafeTopEnd = lua_gettop((l));	\
if (_nLuaSafeTopBegin != _nLuaSafeTopEnd)	\
{                                       \
_ASSERT(0);						\
lua_settop((l), _nLuaSafeTopBegin);	\
}				\

#ifndef PATH_MAX
#define PATH_MAX    1024
#endif

#ifndef GUID_LEN
#define GUID_LEN 64
#endif

#ifndef KG_mkdir
#if (defined(_MSC_VER) || defined(__ICL))
#define KG_mkdir _mkdir
#else   // if linux
#define KG_mkdir(cszDir) mkdir((cszDir), 0755);
#endif
#endif

#ifndef KG_rmdir
#define KG_rmdir rmdir
#endif

#ifndef KG_chdir
#define KG_chdir chdir
#endif



#define KD_MAX_LUA_FUNCTION_NAME_LEN	255	// ∫Ø ˝√˚◊÷◊Ó≥§µƒ≥§∂»£¨”√“ª∏ˆbyte¿¥¥Êµƒ

int GenGuild(char szGuild[], size_t uLen);
#endif /* macros_h */
