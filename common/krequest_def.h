//
//  krequest_def.h
//  thread
//
//  Created by 冯文斌 on 16/12/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef krequest_def_h
#define krequest_def_h

#define RESPOND_FLAG_COMMON	1
#define RESPOND_FLAG_NET_FULL	2
#define RESPOND_FLAG_NET_PART1	3
#define RESPOND_FLAG_NET_PART2	4

#define REQUEST_SET_KEY_REDUN	2

#define REQUEST_KEY_UNDERLINED      "_"
#define REQUEST_KEY_UNDERLINED_LEN   1

enum KE_REQUEST_TYPE
{
    emREQUEST_SET,
    emREQUEST_GET,
    emREQUEST_HSET,
    emREQUEST_HGET,
    emREQUEST_SETS,
    emREQUEST_GETS,
    emREQUEST_HSETS,
    emREQUEST_HGETS,
    emREQUEST_HGETALL,
    emREQUEST_DEL,
    emREQUEST_DELS,
    emREQUEST_HDEL,
    emREQUEST_HDELS,
    emREQUEST_DEL_HASHTABLE,
    emREQUEST_NUM,
    emREQUEST_AUTO_EXPIRE,
};

struct KREQUEST_TYPE
{
    unsigned char byType;
    unsigned char byDBGenre;
    unsigned char byHandleFlag;
    unsigned char byCompleteFlag;
    unsigned int nHash;
};

struct KREQUEST_HEAD : KREQUEST_TYPE
{
    long long lId;
    int nConnectId;
    int nDBType;
    bool bAllowRedisNil;
    int nUid;
    int nEventType;
};

struct KREQUEST_SET : KREQUEST_HEAD
{
    long long lRef;
    int nExpire;
    unsigned int uPrefixLen;
    unsigned int uKeyLen;
    unsigned int uValueLen;
    char	data[1];
};

struct KREQUEST_GET : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uPrefixLen;
    unsigned int uKeyLen;
    char data[1];
};

typedef KREQUEST_GET  KREQUEST_DEL;

struct KREQUEST_HSET : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uTableNameLen;
    unsigned int uHashKeyLen;
    unsigned int uValueLen;
    char data[1];
};

struct KREQUEST_HGET : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uTableNameLen;
    unsigned int uHashKeyLen;
    char data[1];
};

typedef KREQUEST_HGET KREQUEST_HDEL;

struct KREQUEST_KEY_VALUE_DESC
{
    unsigned int uKeyLen;
    unsigned int uValueLen;
    char* pKey;
    char* pValue;
};

struct KREQUEST_SETS : KREQUEST_HEAD
{
    long long lRef;
    int nExpire;
    unsigned int uHashKeyLen;
    int nCount;
    char data[1];
};

struct KREQUEST_KEY_DESC
{
    bool bIsNil;
    unsigned int uKeyLen;
    char* pKey;
};

struct KREQUEST_GETS : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uHashKeyLen;
    int nCount;
    char data[1];
};

typedef KREQUEST_GETS KREQUEST_DELS;

struct KREQUEST_HSETS : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uTableNameLen;
    int nCount;
    char data[1];
};

struct KREQUEST_HGETS : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uTableNameLen;
    int nCount;
    char data[1];
};

typedef KREQUEST_HGETS KREQUEST_HDELS;

struct KREQUEST_HGETALL : KREQUEST_HEAD
{
    long long lRef;
    unsigned int uTableNameLen;
    char data[1];
};

typedef KREQUEST_HGETALL KREQUEST_DELHASHTABLE;

struct KREQUEST_EXPIRE : KREQUEST_TYPE
{
    int nDBType;
    int nExpire;
    const char* pcszTmp;
    char data[1];
};

struct KRESOOND_COMMON
{
    long long lId;
    unsigned char byRequestType;
    unsigned char byHandleFlag;
    unsigned char byReserved2;
    unsigned char byReserved3;
    int nRetType;
    int nParam;
    int nDataLen;
    int nUid;
    int nEventType;
    char data[1];
};

#endif /* krequest_def_h */
