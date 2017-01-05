//
//  db_def.h
//  thread
//
//  Created by 冯文斌 on 16/12/7.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef db_def_h
#define db_def_h

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN            65
#endif

//Redis的set, hset的key的长度不能超过333，mysql的primary key限制长度是1000字节，折成utf8字符是333
#define KD_MAX_DB_KEY_LEN           333

#define DB_FLAG_REDIS               0x01
#define DB_FLAG_MYSQL               0x02

#define DB_WAIT_FLAG_REDIS          0x04
#define DB_WAIT_FLAG_MYSQL          0x08
#define DB_WAIT_FLAG_ALL    (DB_WAIT_FLAG_REDIS | DB_WAIT_FLAG_MYSQL)

#define KD_MAX_DBTYPENAME_LEN       64
#define KD_MAX_DBNAME_LEN           255
#define KD_MAX_TABLENAME_LEN        200

#define  KD_PING_INTERVAL           (3600 * 1000)

//sets和gets一次最多操作的个数
#define KDB_MAX_SETS_GETS_NUM       16

//hash值类型
#define KDB_HASH_LONG               0
#define KDB_HASH_STRING             1

#define KDB_GENRE_COMMON            0   //普通数据类型，set操作redis和mysql都处理，get操作优先处理redis，没有配置redis则从mysql取，只要配置了redis则只从redis读取
#define KDB_GENRE_REDIS             1   //只操作redis
#define KDB_GENRE_MYSQL             2   //只操作mysql
#define KDB_GENRE_REDIS_EXPIRE      3   //针对set操作,redis操作完成返回,get操作会使用KDB_GENRE_REDIS_THEN_MYSQL类型,redis带定时功能,该类型需要在mysql设置完成后给redis数据设置过期时间
#define KDB_GENRE_REDIS_THEN_MYSQL  4   //只针对get操作,取到数据先立即返回,redis找不到key值去mysql取，并将值设置到redis

#define KDB_VALID_GENRE(genre)	((genre) >= KDB_GENRE_COMMON && (genre) <= KDB_GENRE_REDIS_THEN_MYSQL)

#define KDB_REPLY_STRING            1
#define KDB_REPLY_ARRAY             2
#define KDB_REPLY_INTEGER           3
#define KDB_REPLY_NIL               4
#define KDB_REPLY_STATUS            5
#define KDB_REPLY_ERROR             6

struct KNetAddress
{
    char szIp[INET6_ADDRSTRLEN];
    int nPort;
};

#pragma pack(push, 1)

struct KConnectInfo
{
    KConnectInfo()
    {
        nPort = 0;
        szHost[0] = '\0';
        szUser[0] = '\0';
        szPwd[0] = '\0';
    }
    int nPort;
    char szHost[INET6_ADDRSTRLEN];
    char szUser[63];
    char szPwd[64];
};

enum KE_DB_GENRE
{
    emDBGENRE_COMMON =  1,  //能用数据类型, set操作redis和mysql都处理, get操作只处理redis
    emDBGENRE_REDIS =   2,  //只操作redis
    emDBGENRE_MYSQL =   3   //只操作mysql
};

//预定义的数据事件,保持redis和mysql能同步
enum KE_DBEVENT_TYPE
{
    emDBEVENT_TYPE_GET,
    emDBEVENT_TYPE_SET,
    emDBEVENT_TYPE_MGET,
    emDBEVENT_TYPE_MSET,
    emDBEVENT_TYPE_DEL,
    emDBEVENT_TYPE_MDEL,
    emDBEVENT_TYPE_HGETALL, //权限于hash表操作
    emDBEVENT_TYPE_NUM,
};

struct KDB_VALUE
{
    int nSize;
    const char* pStr;
};

struct KDB_KEY_VALUE
{
    unsigned int uKeySize;
    unsigned int uValueSize;
    const char* pKey;
    const char* pValue;
};

#pragma pack(pop)

#endif /* db_def_h */





















