//
//  redis_client.cpp
//  thread
//
//  Created by 冯文斌 on 16/12/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <string.h>
#include <iostream>
#include "db_client_manager.hpp"
#include "redis_client.hpp"
#include "kmacros.h"
#include "file_util.h"
#include "document.h"
#include "game_logic_server.hpp"

#define KD_INIT_NETPACKET_HEAD(data, connectId)	\
data -= sizeof(unsigned char);				\
*(unsigned char*)data = RESPOND_FLAG_NET_FULL;	\
data -= sizeof(int);						\
*(int*)data = connectId;		\
data += sizeof(int) + sizeof(unsigned char)	\


#define KD_INIT_NONFULL_NETPACKET_HEAD(data, connectId)	\
data -= sizeof(unsigned char);				\
*(unsigned char*)data = RESPOND_FLAG_NET_PART1;	\
data -= sizeof(int);						\
*(int*)data = connectId;		\
data += sizeof(int) + sizeof(unsigned char)	\

#define KD_KV_FORMAT	"%b %b "
#define KD_KV_FORMAT_0	KD_KV_FORMAT
#define KD_KV_FORMAT_1	KD_KV_FORMAT_0 KD_KV_FORMAT
#define KD_KV_FORMAT_2	KD_KV_FORMAT_1 KD_KV_FORMAT
#define KD_KV_FORMAT_3	KD_KV_FORMAT_2 KD_KV_FORMAT
#define KD_KV_FORMAT_4	KD_KV_FORMAT_3 KD_KV_FORMAT
#define KD_KV_FORMAT_5	KD_KV_FORMAT_4 KD_KV_FORMAT
#define KD_KV_FORMAT_6	KD_KV_FORMAT_5 KD_KV_FORMAT
#define KD_KV_FORMAT_7	KD_KV_FORMAT_6 KD_KV_FORMAT
#define KD_KV_FORMAT_8	KD_KV_FORMAT_7 KD_KV_FORMAT
#define KD_KV_FORMAT_9	KD_KV_FORMAT_8 KD_KV_FORMAT
#define KD_KV_FORMAT_10	KD_KV_FORMAT_9 KD_KV_FORMAT
#define KD_KV_FORMAT_11	KD_KV_FORMAT_10 KD_KV_FORMAT
#define KD_KV_FORMAT_12	KD_KV_FORMAT_11 KD_KV_FORMAT
#define KD_KV_FORMAT_13	KD_KV_FORMAT_12 KD_KV_FORMAT
#define KD_KV_FORMAT_14	KD_KV_FORMAT_13 KD_KV_FORMAT
#define KD_KV_FORMAT_15	KD_KV_FORMAT_14 KD_KV_FORMAT

#define KD_MSET_FORMAT(index)	"mset " KD_KV_FORMAT_##index##""
#define KD_HMSET_FORMAT(index)	"hmset %b " KD_KV_FORMAT_##index##""

#define KD_KEY_VALUE(pKVDesc, index)		pKVDesc[index].pKey, pKVDesc[index].uKeyLen, pKVDesc[index].pValue, pKVDesc[index].uValueLen
#define KD_KEY_VALUE_0(pKVDesc)		KD_KEY_VALUE(pKVDesc, 0)
#define KD_KEY_VALUE_1(pKVDesc)		KD_KEY_VALUE_0(pKVDesc), KD_KEY_VALUE(pKVDesc, 1)
#define KD_KEY_VALUE_2(pKVDesc)		KD_KEY_VALUE_1(pKVDesc), KD_KEY_VALUE(pKVDesc, 2)
#define KD_KEY_VALUE_3(pKVDesc)		KD_KEY_VALUE_2(pKVDesc), KD_KEY_VALUE(pKVDesc, 3)
#define KD_KEY_VALUE_4(pKVDesc)		KD_KEY_VALUE_3(pKVDesc), KD_KEY_VALUE(pKVDesc, 4)
#define KD_KEY_VALUE_5(pKVDesc)		KD_KEY_VALUE_4(pKVDesc), KD_KEY_VALUE(pKVDesc, 5)
#define KD_KEY_VALUE_6(pKVDesc)		KD_KEY_VALUE_5(pKVDesc), KD_KEY_VALUE(pKVDesc, 6)
#define KD_KEY_VALUE_7(pKVDesc)		KD_KEY_VALUE_6(pKVDesc), KD_KEY_VALUE(pKVDesc, 7)
#define KD_KEY_VALUE_8(pKVDesc)		KD_KEY_VALUE_7(pKVDesc), KD_KEY_VALUE(pKVDesc, 8)
#define KD_KEY_VALUE_9(pKVDesc)		KD_KEY_VALUE_8(pKVDesc), KD_KEY_VALUE(pKVDesc, 9)
#define KD_KEY_VALUE_10(pKVDesc)	KD_KEY_VALUE_9(pKVDesc), KD_KEY_VALUE(pKVDesc, 10)
#define KD_KEY_VALUE_11(pKVDesc)	KD_KEY_VALUE_10(pKVDesc), KD_KEY_VALUE(pKVDesc, 11)
#define KD_KEY_VALUE_12(pKVDesc)	KD_KEY_VALUE_11(pKVDesc), KD_KEY_VALUE(pKVDesc, 12)
#define KD_KEY_VALUE_13(pKVDesc)	KD_KEY_VALUE_12(pKVDesc), KD_KEY_VALUE(pKVDesc, 13)
#define KD_KEY_VALUE_14(pKVDesc)	KD_KEY_VALUE_13(pKVDesc), KD_KEY_VALUE(pKVDesc, 14)
#define KD_KEY_VALUE_15(pKVDesc)	KD_KEY_VALUE_14(pKVDesc), KD_KEY_VALUE(pKVDesc, 15)

#define KD_KEY_VALUE__(pKVDesc, index)	KD_KEY_VALUE_##index##(pKVDesc)


#define KD_KEY_FORMAT	"%b "
#define KD_KEY_FORMAT_0	KD_KEY_FORMAT
#define KD_KEY_FORMAT_1		KD_KEY_FORMAT_0 KD_KEY_FORMAT
#define KD_KEY_FORMAT_2		KD_KEY_FORMAT_1 KD_KEY_FORMAT
#define KD_KEY_FORMAT_3		KD_KEY_FORMAT_2 KD_KEY_FORMAT
#define KD_KEY_FORMAT_4		KD_KEY_FORMAT_3 KD_KEY_FORMAT
#define KD_KEY_FORMAT_5		KD_KEY_FORMAT_4 KD_KEY_FORMAT
#define KD_KEY_FORMAT_6		KD_KEY_FORMAT_5 KD_KEY_FORMAT
#define KD_KEY_FORMAT_7		KD_KEY_FORMAT_6 KD_KEY_FORMAT
#define KD_KEY_FORMAT_8		KD_KEY_FORMAT_7 KD_KEY_FORMAT
#define KD_KEY_FORMAT_9		KD_KEY_FORMAT_8 KD_KEY_FORMAT
#define KD_KEY_FORMAT_10	KD_KEY_FORMAT_9 KD_KEY_FORMAT
#define KD_KEY_FORMAT_11	KD_KEY_FORMAT_10 KD_KEY_FORMAT
#define KD_KEY_FORMAT_12	KD_KEY_FORMAT_11 KD_KEY_FORMAT
#define KD_KEY_FORMAT_13	KD_KEY_FORMAT_12 KD_KEY_FORMAT
#define KD_KEY_FORMAT_14	KD_KEY_FORMAT_13 KD_KEY_FORMAT
#define KD_KEY_FORMAT_15	KD_KEY_FORMAT_14 KD_KEY_FORMAT

#define KD_MGET_FORMAT(index)	"mget " KD_KEY_FORMAT_##index##""
#define KD_HMGET_FORMAT(index)	"hmget %b " KD_KEY_FORMAT_##index##""
#define KD_DEL_FORMAT(index)	"del " KD_KEY_FORMAT_##index##""
#define KD_HDEL_FORMAT(index)	"hdel %b " KD_KEY_FORMAT_##index##""

#define KD_KEY(pKeyDesc, index)		pKeyDesc[index].pKey, pKeyDesc[index].uKeyLen
#define KD_KEY_0(pKeyDesc)		KD_KEY(pKeyDesc, 0)
#define KD_KEY_1(pKeyDesc)		KD_KEY_0(pKeyDesc), KD_KEY(pKeyDesc, 1)
#define KD_KEY_2(pKeyDesc)		KD_KEY_1(pKeyDesc), KD_KEY(pKeyDesc, 2)
#define KD_KEY_3(pKeyDesc)		KD_KEY_2(pKeyDesc), KD_KEY(pKeyDesc, 3)
#define KD_KEY_4(pKeyDesc)		KD_KEY_3(pKeyDesc), KD_KEY(pKeyDesc, 4)
#define KD_KEY_5(pKeyDesc)		KD_KEY_4(pKeyDesc), KD_KEY(pKeyDesc, 5)
#define KD_KEY_6(pKeyDesc)		KD_KEY_5(pKeyDesc), KD_KEY(pKeyDesc, 6)
#define KD_KEY_7(pKeyDesc)		KD_KEY_6(pKeyDesc), KD_KEY(pKeyDesc, 7)
#define KD_KEY_8(pKeyDesc)		KD_KEY_7(pKeyDesc), KD_KEY(pKeyDesc, 8)
#define KD_KEY_9(pKeyDesc)		KD_KEY_8(pKeyDesc), KD_KEY(pKeyDesc, 9)
#define KD_KEY_10(pKeyDesc)		KD_KEY_9(pKeyDesc), KD_KEY(pKeyDesc, 10)
#define KD_KEY_11(pKeyDesc)		KD_KEY_10(pKeyDesc), KD_KEY(pKeyDesc, 11)
#define KD_KEY_12(pKeyDesc)		KD_KEY_11(pKeyDesc), KD_KEY(pKeyDesc, 12)
#define KD_KEY_13(pKeyDesc)		KD_KEY_12(pKeyDesc), KD_KEY(pKeyDesc, 13)
#define KD_KEY_14(pKeyDesc)		KD_KEY_13(pKeyDesc), KD_KEY(pKeyDesc, 14)
#define KD_KEY_15(pKeyDesc)		KD_KEY_14(pKeyDesc), KD_KEY(pKeyDesc, 15)

#define KD_KEY__(pKeyDesc, index)	KD_KEY_##index##(pKeyDesc)

using namespace std;
using namespace rapidjson;

KRedisClient::KRedisClient()
{
    m_nQueryCount = 0;
    m_ulAverageUSec = 0;
}

KRedisClient::~KRedisClient()
{
    
}

bool KRedisClient::Init(int nDBType)
{
    string sz_config;
    bool is_ok = g_pFileUtil->ReadFile("config.json", sz_config);
    if(!is_ok)
    {
        cout << "read config failed." << endl;
        return false;
    }
    
    Document json_doc;
    json_doc.Parse(sz_config.c_str());
    
    const Value& server_config = json_doc["redis"];
    string szIp = server_config["ip"].GetString();
    int nPort = server_config["port"].GetInt();
    
    KConnectInfo connectInfo = *new KConnectInfo();
    connectInfo.nPort = nPort;
    memcpy(connectInfo.szHost, szIp.c_str(), szIp.length());
    std::vector<KConnectInfo> vecInfo;
    vecInfo.push_back(connectInfo);
    
    if(vecInfo.size() == 0)
    {
        _ASSERT(false);
    }
    return KDBClient::Init(false, "", vecInfo);;
}

bool KRedisClient::UnInit()
{
    KDBClient::UnInit();
    for (std::vector<redisContext*>::iterator it = m_vecRedisContext.begin(); it != m_vecRedisContext.end(); ++it)
    {
        if (*it)
        {
            redisFree(*it);
        }
    }
    m_vecRedisContext.clear();
    return true;
}

bool KRedisClient::ConnectDB()
{
    bool bResult = false;
    for (unsigned int i = 0; i < m_vecConnectInfo.size(); i++)
    {
        KConnectInfo& sConnectInfo = m_vecConnectInfo[i];
        redisContext* pRedisContext = redisConnect(sConnectInfo.szHost, sConnectInfo.nPort);
        if (pRedisContext != NULL && pRedisContext->err)
        {
            redisFree(pRedisContext);
            pRedisContext = NULL;
            goto Exit0;
        }
        m_vecRedisContext.push_back(pRedisContext);
    }
    
    m_bDatabaseLost = false;
    bResult = true;
Exit0:
    if (!bResult)
    {
        for (std::vector<redisContext*>::iterator it = m_vecRedisContext.begin(); it != m_vecRedisContext.end(); ++it)
        {
            redisFree(*it);
        }
        m_vecRedisContext.clear();
    }
    return bResult;
}

bool KRedisClient::Reconnect(int nIndex)
{
    int nTryTimes = 0;
    const KConnectInfo& sConnectInfo = m_vecConnectInfo[nIndex];
    redisContext* pRedisContext = m_vecRedisContext[nIndex];
    //KGLOG_PROCESS_ERROR(m_bDatabaseLost);
    if (pRedisContext)
    {
        redisFree(pRedisContext);
        pRedisContext = NULL;
        m_vecRedisContext[nIndex] = NULL;
    }
    do
    {
        if (nTryTimes > m_nRedisMaxReconnectTime)
        {
            //KGLogPrintf(KGLOG_ERR, "RedisClient[%s:%d] reconnect fail because reconnect too many times[%s:%d]\n", m_szDBName, nIndex, sConnectInfo.szHost, sConnectInfo.nPort);
            //KGLOG_CONFIRM_BREAK(false);
        }
        nTryTimes++;
        //KGLogPrintf(KGLOG_ERR, "RedisClient[%s:%d] try to reconnect[%s:%d]...\n", m_szDBName, nIndex, sConnectInfo.szHost, sConnectInfo.nPort);
        if (nTryTimes > 1 && m_nRedisReconnectSleepTime > 0)
        {
            KSLEEP(m_nRedisReconnectSleepTime);
        }
        pRedisContext = redisConnect(sConnectInfo.szHost, sConnectInfo.nPort);
        if (!pRedisContext)
        {
            //KGLogPrintf(KGLOG_ERR, "RedisClient[%s:%d]  Redis reconnect failure!!!\n", m_szDBName, nIndex);
            //KGLOG_CONFIRM_CONTINUE(false);
        }
        if (pRedisContext->err)
        {
            //KGLogPrintf(KGLOG_ERR, "RedisClient[%s:%d]  ReConnect Redis Erro: %s\n", m_szDBName, nIndex, pRedisContext->errstr);
            redisFree(pRedisContext);
            pRedisContext = NULL;
            //KGLOG_CONFIRM_CONTINUE(false);
        }
        m_vecRedisContext[nIndex] = pRedisContext;
        //KGLogPrintf(KGLOG_INFO, ".............RedisClient[%s:%d] [%s:%d] Reconnect Redis Success................", m_szDBName, nIndex, sConnectInfo.szHost, sConnectInfo.nPort);
        m_bDatabaseLost = false;
        break;
    } while (true);
Exit0:	
    return !m_bDatabaseLost;
}

bool KRedisClient::OnRequest(IKG_Buffer* pBuffer)
{
    bool bResult = false;
    const KREQUEST_TYPE* pRequest = (const KREQUEST_TYPE*)pBuffer->GetData();
    int nIndex = pRequest->nHash % m_vecRedisContext.size();
    switch (pRequest->byType)
    {
        case emREQUEST_SET:
            bResult = OnRequestSet(nIndex, (const KREQUEST_SET*)pRequest);
            break;
        case emREQUEST_GET:
            bResult = OnRequestGet(nIndex, (const KREQUEST_GET*)pRequest);
            break;
        case emREQUEST_AUTO_EXPIRE:
            bResult = OnRequestExpire(nIndex, (const KREQUEST_EXPIRE*)pRequest);
            break;
        case emREQUEST_HSET:
            bResult = OnRequestHSet(nIndex, (const KREQUEST_HSET*)pRequest);
            break;;
        case emREQUEST_HGET:
            bResult = OnRequestHGet(nIndex, (const KREQUEST_HGET*)pRequest);
            break;
        case emREQUEST_DEL:
            bResult = OnRequestDel(nIndex, (const KREQUEST_DEL*)pRequest);
            break;
        case emREQUEST_HDEL:
            bResult = OnRequestHDel(nIndex, (const KREQUEST_HDEL*)pRequest);
            break;
        /*case emREQUEST_SETS:
            bResult = OnRequestSets(nIndex, (const KREQUEST_SETS*)pRequest);
            break;
        case emREQUEST_GETS:
            bResult = OnRequestGets(nIndex, (const KREQUEST_GETS*)pRequest);
            break;
        case emREQUEST_HSETS:
            bResult = OnRequestHSets(nIndex, (const KREQUEST_HSETS*)pRequest);
            break;
        case emREQUEST_HGETS:
            bResult = OnRequestHGets(nIndex, (const KREQUEST_HGETS*)pRequest);
            break;
        case emREQUEST_HGETALL:
            bResult = OnRequestHGetAll(nIndex, (const KREQUEST_HGETALL*)pRequest);
            break;
        case emREQUEST_DELS:
            bResult = OnRequestDels(nIndex, (const KREQUEST_DELS*)pRequest);
            break;
        case emREQUEST_HDELS:
            bResult = OnRequestHDels(nIndex, (const KREQUEST_HDELS*)pRequest);
            break;
        case emREQUEST_DEL_HASHTABLE:
            bResult = OnRequestDelHashTable(nIndex, (const KREQUEST_DELHASHTABLE*)pRequest);
            break;*/
        default:
            _ASSERT(false);
            break;
    }
    return bResult;
}


bool KRedisClient::OnResponsed(IKG_Buffer* pBuffer)
{
    KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
    std::cout << "onResponse....event type:" << pResond->nEventType << std::endl;
    GameLogicServer* pInstance = GameLogicServer::GetInstance();
    pInstance->OnDBResponse(pResond);
    return true;
}

redisReply* KRedisClient::RedisCommand(int nIndex, const char* format, ...)
{
    redisReply *pReply = NULL;
    unsigned int uElapse = 0;
    int nIOTryp = 0;
    int nTryNum = 0;
    ++m_nQueryCount;
    redisContext* pRedisContext = m_vecRedisContext[nIndex];
    va_list ap;
    va_start(ap, format);
    m_cTimer.Start();
    do
    {
        pReply = static_cast<redisReply*>(redisvCommand(pRedisContext, format, ap));
        if (!pReply)
        {
            //KGLOG_PROCESS_ERROR(nTryNum < 3);
            nTryNum++;
            m_bDatabaseLost = true;
            bool bRet = Reconnect(nIndex);
            if (bRet)
            {
                pRedisContext = m_vecRedisContext[nIndex];
                continue;
            }
            else
            {
                exit(0);
            }
            //KGLOG_PROCESS_ERROR(false);
        }
        else if (pReply->type == REDIS_REPLY_ERROR && pRedisContext->err == REDIS_ERR_IO && nIOTryp < 5)
        {
            //KGLogPrintf(KGLOG_ERR, "RedisClient[%s:%d] RedisCommand[%s] IO Error:%s\n", m_szDBName, nIndex, format, pReply->str);
            nIOTryp++;
            freeReplyObject(pReply);
            pReply = NULL;
            continue;
        }
        break;
    } while (true);
    
Exit0:
    uElapse = m_cTimer.GetElapseMicrosecond();
    m_ulAverageUSec += uElapse;
    if (m_nQueryCount % m_nRedisLogFrequency == 0)
    {
        //KGLogPrintf(KGLOG_INFO, "RedisClient[%s:%d] complete query[%d] , cost time[%u]usec avg[%u]usec\n", m_szDBName, nIndex, m_nQueryCount, uElapse, m_ulAverageUSec / m_nQueryCount);
    }
    va_end(ap);
    return pReply;
}

IKG_Buffer* KRedisClient::GenCommonRespond(long long lId, unsigned char byType, redisReply* pReply)
{
    _ASSERT(pReply);
    IKG_Buffer* pBuffer = NULL;
    if (pReply->type == REDIS_REPLY_ERROR)
    {
        pBuffer = DB_MemoryCreateBuffer((int)(sizeof(KRESOOND_COMMON) + pReply->len));
        KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
        pResond->lId = lId;
        pResond->byHandleFlag = DB_WAIT_FLAG_REDIS;
        pResond->byRequestType = byType;
        pResond->nRetType = pReply->type;
        pResond->nParam = 0;
        pResond->nDataLen = (int)pReply->len;
        memcpy(pResond->data, pReply->str, pReply->len);
        pResond->data[pReply->len] = '\0';
    }
    else if (pReply->type == REDIS_REPLY_ARRAY)
    {
        pBuffer = DB_MemoryCreateBuffer((int)(sizeof(KRESOOND_COMMON) + pReply->elements * sizeof(int) - 1));
        KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
        pResond->lId = lId;
        pResond->byHandleFlag = DB_WAIT_FLAG_REDIS;
        pResond->byRequestType = byType;
        pResond->nRetType = pReply->type;
        pResond->nParam = (int)pReply->elements;
        pResond->nDataLen = 0;
        int* pnType = (int*)pResond->data;
        for (int i = 0; i < (int)pReply->elements; i++)
        {
            *pnType++ = pReply->element[i]->type;
        }
    }
    else
    {
        pBuffer = DB_MemoryCreateBuffer((int)(sizeof(KRESOOND_COMMON) + pReply->len));
        KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
        pResond->lId = lId;
        pResond->byHandleFlag = DB_WAIT_FLAG_REDIS;
        pResond->byRequestType = byType;
        pResond->nRetType = pReply->type;
        pResond->nParam = 0;
        pResond->nDataLen =0;
        pResond->nUid = 100001;
        pResond->nEventType = 1;
        pResond->nDataLen = (int)pReply->len;
        memcpy(pResond->data, pReply->str, pReply->len);
        pResond->data[pReply->len] = '\0';
    }
    char* pData = (char*)pBuffer->GetData();
    pData -= sizeof(unsigned char);
    *(unsigned char*)pData = RESPOND_FLAG_COMMON;
    return pBuffer;
}

bool KRedisClient::OnRequestSet(int nIndex, const KREQUEST_SET* pRequestSet)
{
    bool bResult = false;
    //bool bRet = false;
    redisReply* pReply = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    size_t uPrefixLen = pRequestSet->uPrefixLen;
    size_t uKeyLen = pRequestSet->uKeyLen;
    size_t uValueLen = pRequestSet->uValueLen;
    size_t uSetKeyLen = uPrefixLen + uKeyLen + REQUEST_KEY_UNDERLINED_LEN;
    pReply = RedisCommand(nIndex, "set %b %b", pRequestSet->data, uSetKeyLen , pRequestSet->data + uSetKeyLen, uValueLen);
    
    //同步到数据库里
    if(pReply!=NULL)
    {
        if(pReply != NULL)
        {
            size_t nSize = sizeof(KREQUEST_HSET) + uSetKeyLen + uValueLen;
            IKG_Buffer* pBuffer = DB_MemoryCreateBuffer((int)nSize);
            KREQUEST_SET* pRequest = (KREQUEST_SET*)pBuffer->GetData();
            memcpy(pRequest, pRequestSet, pBuffer->GetSize());
            g_pDBClientMgr->PushMysqlRequest(nIndex + 1, pBuffer);
        }
    }
    
    pCommonBuffer = GenCommonRespond(pRequestSet->lId, pRequestSet->byType, pReply);
    PushRespond(pCommonBuffer);
    bResult = true;
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}


bool KRedisClient::OnRequestGet(int nIndex, const KREQUEST_GET* pRequestGet)
{
    bool bResult = false;
    redisReply* pReply = NULL;
    IKG_Buffer* pPackatBuffer = NULL;
    size_t uPrefixLen = pRequestGet->uPrefixLen;
    size_t uKeyLen = pRequestGet->uKeyLen;
    size_t uGetKeyLen = uPrefixLen + uKeyLen + REQUEST_KEY_UNDERLINED_LEN;
    pReply = RedisCommand(nIndex, "get %b", pRequestGet->data, uGetKeyLen);
    
    //从数据库中读取出来
    if (pReply->type == REDIS_REPLY_NIL && !pRequestGet->bAllowRedisNil)
    {
        size_t nSize = sizeof(KREQUEST_GET) + uGetKeyLen;
        IKG_Buffer* pBuffer = DB_MemoryCreateBuffer((int)nSize);
        KREQUEST_GET* pRequest = (KREQUEST_GET*)pBuffer->GetData();
        memcpy(pRequest, pRequestGet, pBuffer->GetSize());
        pPackatBuffer = g_pDBClientMgr->RequestMySQLQuery(nIndex + 1, pBuffer);
        PushRespond(pPackatBuffer);
    }
    
    //返回到调用处,处理调用结果
    bResult = true;
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    
    SAFE_RELEASE(pPackatBuffer);
    return bResult;
}

bool KRedisClient::OnRequestExpire(int nIndex, const KREQUEST_EXPIRE* pRequestExpire)
{
    bool bResult = false;
    redisReply* pReply = RedisCommand(nIndex, "expire %s %d", pRequestExpire->data, pRequestExpire->nExpire);
    bResult = true;
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    return bResult;
}


bool KRedisClient::OnRequestHSet(int nIndex, const KREQUEST_HSET* pRequestHSet)
{
    bool bResult = false;
    redisReply* pReply = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    
    size_t uTableNameLen = pRequestHSet->uTableNameLen;
    size_t uHashKeyLen = pRequestHSet->uHashKeyLen;
    size_t uValueLen = pRequestHSet->uValueLen;
    pReply = RedisCommand(nIndex, "hset %b %b %b",
                          pRequestHSet->data, uTableNameLen,
                          pRequestHSet->data + uTableNameLen, uHashKeyLen,
                          pRequestHSet->data + uTableNameLen + uHashKeyLen, uValueLen);
    //同步到数据库
    if(pReply != NULL)
    {
        size_t nSize = sizeof(KREQUEST_HSET) + uTableNameLen + uHashKeyLen + uValueLen;
        IKG_Buffer* pBuffer = DB_MemoryCreateBuffer((int)nSize);
        KREQUEST_HSET* pRequest = (KREQUEST_HSET*)pBuffer->GetData();
        memcpy(pRequest, pRequestHSet, pBuffer->GetSize());
        g_pDBClientMgr->PushMysqlRequest(nIndex + 1, pBuffer);
    }
    
    pCommonBuffer = GenCommonRespond(pRequestHSet->lId, pRequestHSet->byType, pReply);
    PushRespond(pCommonBuffer);
    bResult = true;
    
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KRedisClient::OnRequestHGet(int nIndex, const KREQUEST_HGET* pRequestHGet)
{
    bool bResult = false;
    redisReply* pReply = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    pReply = RedisCommand(nIndex, "hget %b %b", pRequestHGet->data, (size_t)pRequestHGet->uTableNameLen, pRequestHGet->data + pRequestHGet->uTableNameLen, (size_t)pRequestHGet->uHashKeyLen);
    if (pReply->type == REDIS_REPLY_NIL && !pRequestHGet->bAllowRedisNil)
    {
        //to do:从数据库里读取出来
        goto Exit1;
    }
    
Exit1:
    pCommonBuffer = GenCommonRespond(pRequestHGet->lId, pRequestHGet->byType, pReply);
    PushRespond(pCommonBuffer);
    bResult = true;
    
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KRedisClient::OnRequestDel(int nIndex, const KREQUEST_DEL* pRequestDel)
{
    bool bResult = false;
    redisReply* pReply = NULL;
    size_t uPrefixLen = pRequestDel->uPrefixLen;
    size_t uKeyLen = pRequestDel->uKeyLen;
    size_t uDelKeyLen = uPrefixLen + uKeyLen + REQUEST_KEY_UNDERLINED_LEN;
    pReply = RedisCommand(nIndex, "del %b", pRequestDel->data, uDelKeyLen);
    
    //同步到数据库里
    if(pReply!=NULL)
    {
        if(pReply != NULL)
        {
            size_t nSize = sizeof(KREQUEST_DEL) + uDelKeyLen;
            IKG_Buffer* pBuffer = DB_MemoryCreateBuffer((int)nSize);
            KREQUEST_DEL* pRequest = (KREQUEST_DEL*)pBuffer->GetData();
            memcpy(pRequest, pRequestDel, pBuffer->GetSize());
            g_pDBClientMgr->PushMysqlRequest(nIndex + 1, pBuffer);
        }
    }
    
    bResult = true;
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    
    return bResult;
}

bool KRedisClient::OnRequestHDel(int nIndex, const KREQUEST_HDEL* pRequestHDel)
{
    bool bResult = false;
    redisReply* pReply = NULL;
    size_t uTableNameLen = pRequestHDel->uTableNameLen;
    size_t uHashKeyLen = pRequestHDel->uHashKeyLen;
    size_t uHDelKeyLen = uTableNameLen + uHashKeyLen;
    pReply = RedisCommand(nIndex, "hdel %b %b", pRequestHDel->data, uTableNameLen, pRequestHDel->data + uTableNameLen, uHashKeyLen);
    
    //同步到数据库
    if(pReply!=NULL)
    {
        size_t nSize = sizeof(KREQUEST_HDEL) + uHDelKeyLen;
        IKG_Buffer* pBuffer = DB_MemoryCreateBuffer((int)nSize);
        KREQUEST_HDEL* pRequest = (KREQUEST_HDEL*)pBuffer->GetData();
        memcpy(pRequest, pRequestHDel, pBuffer->GetSize());
        g_pDBClientMgr->PushMysqlRequest(nIndex + 1, pBuffer);
    }
    
    bResult = true;
Exit0:
    if (pReply)
    {
        freeReplyObject(pReply);
    }
    
    return bResult;
}
