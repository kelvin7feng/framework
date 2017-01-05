//
//  mysql_client.cpp
//  thread
//
//  Created by 冯文斌 on 16/12/30.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "mysql_client.hpp"
#include "kmacros.h"
#include <errmsg.h>
#include "krequest_def.h"
#include "mysqld_error.h"
#include "file_util.h"
#include "document.h"
#include <string.h>
#include <iostream>

#define KD_INIT_NETPACKET_HEAD(data, connectId)	\
data -= sizeof(unsigned char);				\
*(unsigned char*)data = RESPOND_FLAG_NET_FULL;	\
data -= sizeof(int);						\
*(int*)data = connectId;		\
data += sizeof(int) + sizeof(unsigned char)	\

#define KD_INIT_NONFULL_NETPACKET_HEAD(data, connectId)	\
data -= sizeof(unsigned char);				\
*(unsigned char*)data = RESPOND_FLAG_NET_PART2;	\
data -= sizeof(int);						\
*(int*)data = connectId;		\
data += sizeof(int) + sizeof(unsigned char)	\


#define GLOBAL_TABLE_NAME_PREFIX	"gt_"
#define GLOBAL_TABLE_NAME_PREFIX_LEN	3

#define HASH_TABLE_NAME_PREFIX	"ht_"
#define HASH_TABLE_NAME_PREFIX_LEN	3

using namespace std;
using namespace rapidjson;

KMysqlClient::KMysqlClient()
{
    m_uBuffLen = 0;
    m_pSQLBuff = 0;
}

KMysqlClient::~KMysqlClient()
{
    SAFE_DELETE_ARRAY(m_pSQLBuff);
    _ASSERT(m_pSQLBuff == NULL);
    _ASSERT(m_vecMysql.size() == 0);
}

bool KMysqlClient::Init(int nDBType)
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
    
    const Value& server_config = json_doc["mysql"];
    string sz_ip = server_config["ip"].GetString();
    int port = server_config["port"].GetInt();
    string sz_user = server_config["user"].GetString();
    string sz_password = server_config["password"].GetString();
    string sz_scheme = server_config["scheme"].GetString();
    
    m_uBuffLen = 1024 * 50;
    m_pSQLBuff = new char[m_uBuffLen];
    KConnectInfo connectInfo = *new KConnectInfo();
    connectInfo.nPort = port;
    memcpy(connectInfo.szHost, sz_ip.c_str(), sz_ip.length());
    memcpy(connectInfo.szUser, sz_user.c_str(), sz_user.length());
    memcpy(connectInfo.szPwd, sz_password.c_str(), sz_password.length());
    std::vector<KConnectInfo> vecInfo;
    vecInfo.push_back(connectInfo);
    
    if(vecInfo.size() == 0)
    {
        _ASSERT(false);
    }
    return KDBClient::Init(false, sz_scheme.c_str(), vecInfo);;
}

bool KMysqlClient::UnInit()
{
    KDBClient::UnInit();
    for (std::vector<MYSQL*>::iterator it = m_vecMysql.begin(); it != m_vecMysql.end(); ++it)
    {
        if (*it)
        {
            mysql_close(*it);
        }
    }
    m_vecMysql.clear();
    for (std::vector<SET_TABLENAME*>::iterator itSet = m_vecTableName.begin(); itSet != m_vecTableName.end(); ++itSet)
    {
        SET_TABLENAME* pNameSet = *itSet;
        for (SET_TABLENAME::iterator it = pNameSet->begin(); it != pNameSet->end(); ++it)
        {
            delete[] it->pcszName;
        }
        pNameSet->clear();
        delete pNameSet;
    }
    m_vecTableName.clear();
    m_uBuffLen = 0;
    SAFE_DELETE_ARRAY(m_pSQLBuff);
    return true;
}

bool KMysqlClient::CreateDatabase(int nIndex, const char cszDatabaseName[KD_MAX_DBNAME_LEN], int nCreateFlag )
{
    bool bResult = 0;
    int nRetCode = 0;
    char szQuery[sizeof("CREATE DATABASE IF NOT EXISTS ") + KD_MAX_DBNAME_LEN];
    //KGLOG_PROCESS_ERROR(nIndex >= 0 && nIndex < (int)m_vecMysql.size());
    //KGLOG_PROCESS_ERROR(cszDatabaseName);
    nRetCode = (int)strlen(cszDatabaseName);
    //KGLOG_PROCESS_ERROR(nRetCode > 0 && nRetCode < KD_MAX_DBNAME_LEN);
    if (nCreateFlag & emKDBCREATE_IF_NOT_EXIST)
    {
        nRetCode = sprintf(
                           szQuery, "CREATE DATABASE IF NOT EXISTS %s", cszDatabaseName
                           );
    }
    else
    {
        nRetCode = sprintf(
                           szQuery, "CREATE DATABASE %s", cszDatabaseName
                           );
    }
    //KGLOG_PROCESS_ERROR(nRetCode != -1);
    nRetCode = _DoQuery(nIndex, szQuery, nRetCode);
    //KGLOG_PROCESS_ERROR(nRetCode);
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::SetCurrentDatabase(MYSQL* pMysql, const char cszDatabaseName[KD_MAX_DBNAME_LEN])
{
    bool bResult = 0;
    int nRetCode = 0;
    //KGLOG_PROCESS_ERROR(pMysql);
    //KGLOG_PROCESS_ERROR(cszDatabaseName);
    nRetCode = (int)strlen(cszDatabaseName);
    //KGLOG_PROCESS_ERROR(nRetCode > 0);
    //KGLOG_PROCESS_ERROR(nRetCode < KD_MAX_DBNAME_LEN);
    // set current database
    nRetCode = mysql_select_db(pMysql, cszDatabaseName);
    if (nRetCode != 0)
    {
        //KG_ERROR(mysql_error(pMysql));
        //KGLOG_PROCESS_ERROR(false);
    }
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::ConnectDB()
{
    bool bResult = false;
    const char* pcszLocalhost = "localhost";
    char szDBName[128];
    for (unsigned int i = 0; i < m_vecConnectInfo.size(); i++)
    {
        char value = 1;
        KConnectInfo& sConnectInfo = m_vecConnectInfo[i];
        bool bRetCode;
        const char* pcszHost = pcszLocalhost;
        MYSQL* pConnectRet = NULL;
        MYSQL* pMysql = mysql_init(NULL);
        
        if (strcmp(sConnectInfo.szHost, "127.0.0.1") != 0)
        {
            pcszHost = sConnectInfo.szHost;
        }
        pConnectRet = mysql_real_connect(pMysql, pcszHost, sConnectInfo.szUser, sConnectInfo.szPwd, m_szDBName, (unsigned int)sConnectInfo.nPort, NULL, CLIENT_FOUND_ROWS);
        if (pConnectRet != pMysql)
        {
            //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] Connect Error:%s", m_szDBName, i, mysql_error(pMysql));
            //KGLOG_PROCESS_ERROR(false);
            exit(1);
        }
        mysql_options(pMysql, MYSQL_OPT_RECONNECT, &value);
        m_vecMysql.push_back(pMysql);
        m_vecTableName.push_back(new SET_TABLENAME());
        
        snprintf(szDBName, sizeof(szDBName), "%s_%d", m_szDBName, i + 1);
        bRetCode = CreateDatabase(i, szDBName, emKDBCREATE_IF_NOT_EXIST);
        //KGLOG_PROCESS_ERROR(bRetCode);
        //bRetCode = SetCurrentDatabase(pMysql, szDBName);
        //KGLOG_PROCESS_ERROR(bRetCode);
    }
    m_bDatabaseLost = false;
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::Reconnect(int nIndex)
{
    int nTryTimes = 0;
    const KConnectInfo& sConnectInfo = m_vecConnectInfo[nIndex];
    MYSQL* pMysql = m_vecMysql[nIndex];
    char szDBName[128];
    snprintf(szDBName, sizeof(szDBName), "%s_%d", m_szDBName, nIndex + 1);
    const char* pcszHost = "localhost";
    if (strcmp(sConnectInfo.szHost, "127.0.0.1") != 0)
    {
        pcszHost = sConnectInfo.szHost;
    }
    do
    {
        MYSQL* pConnectRet = NULL;
        char value = 1;
        if (nTryTimes > m_nMysqlMaxReconnectTime)
        {
            //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s] reconnect fail because reconnect too many times[%d:%s:%d]\n", szDBName, nIndex, sConnectInfo.szHost, sConnectInfo.nPort);
            //KGLOG_CONFIRM_BREAK(false);
        }
        nTryTimes++;
        //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s] try to reconnect[%d:%s:%d]...\n", szDBName, nIndex, sConnectInfo.szHost, sConnectInfo.nPort);
        if (nTryTimes > 1 && m_nMysqlReconnectSleepTime > 0)
        {
            KSLEEP(m_nMysqlReconnectSleepTime);
        }
        pConnectRet = mysql_real_connect(pMysql, pcszHost, sConnectInfo.szUser, sConnectInfo.szPwd, szDBName, (unsigned int)sConnectInfo.nPort, NULL, CLIENT_FOUND_ROWS);
        if (pConnectRet != pMysql)
        {
            //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] Reconnect Error: %s\n", szDBName, nIndex, mysql_error(pMysql));
            //KGLOG_CONFIRM_BREAK(false);
        }
        mysql_options(pMysql, MYSQL_OPT_RECONNECT, &value);
        //KGLogPrintf(KGLOG_INFO, "MysqlClient[%s:%d] Reconnect Success!!!!\n", szDBName, nIndex);
        m_bDatabaseLost = false;
    } while (false);
Exit0:
    return !m_bDatabaseLost;
}

bool KMysqlClient::OnRequest(IKG_Buffer* pBuffer)
{
    bool bResult = false;
    const KREQUEST_TYPE* pRequest = (const KREQUEST_TYPE*)pBuffer->GetData();
    int nIndex = pRequest->nHash % m_vecMysql.size();
    switch (pRequest->byType)
    {
        case emREQUEST_SET:
            bResult = OnRequestSet(nIndex, (const KREQUEST_SET*)pRequest);
            break;
        case emREQUEST_HSET:
            bResult = OnRequestHSet(nIndex, (const KREQUEST_HSET*)pRequest);
            break;
        case emREQUEST_SETS:
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
        case emREQUEST_DEL:
            bResult = OnRequestDel(nIndex, (const KREQUEST_DEL*)pRequest);
            break;
        case emREQUEST_DELS:
            bResult = OnRequestDels(nIndex, (const KREQUEST_DELS*)pRequest);
            break;
        case emREQUEST_HDEL:
            bResult = OnRequestHDel(nIndex, (const KREQUEST_HDEL*)pRequest);
            break;
        case emREQUEST_HDELS:
            bResult = OnRequestHDels(nIndex, (const KREQUEST_HDELS*)pRequest);
            break;
        case emREQUEST_DEL_HASHTABLE:
            bResult = OnRequestDelHashTable(nIndex, (const KREQUEST_DELHASHTABLE*)pRequest);
            break;
        default:
            _ASSERT(false);
            break;
    }
    return bResult;
}

IKG_Buffer* KMysqlClient::OnRequestQuery(IKG_Buffer* pBuffer)
{
    const KREQUEST_TYPE* pRequest = (const KREQUEST_TYPE*)pBuffer->GetData();
    int nIndex = pRequest->nHash % m_vecMysql.size();
    IKG_Buffer* pPackBuffer = NULL;
    switch (pRequest->byType)
    {
        case emREQUEST_GET:
            pPackBuffer = OnRequestGet(nIndex, (const KREQUEST_GET*)pRequest);
            break;
        case emREQUEST_HGET:
            pPackBuffer = OnRequestHGet(nIndex, (const KREQUEST_HGET*)pRequest);
            break;
        default:
            _ASSERT(false);
            break;
    }
    return pPackBuffer;
}

bool KMysqlClient::OnResponsed(IKG_Buffer* pBuffer)
{
    //g_pDBClientMgr->ProcessRespond(pBuffer);
    return true;
}

void KMysqlClient::Ping()
{
    for (unsigned int i = 0; i < m_vecMysql.size(); i++)
    {
        MYSQL* pMysql = m_vecMysql[i];
        if (pMysql)
        {
            int nRet = mysql_ping(pMysql);
            if (nRet != 0)
            {
                //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] ping fail!!!!\n", m_szDBName, i, mysql_error(pMysql));
            }
        }
    }
    
}

bool KMysqlClient::CheckAndCreateRedisHashTable(int nIndex, const KTableName& rsTableName)
{
    bool bResult = false;
    //KGLOG_PROCESS_ERROR(rsTableName.pcszName);
    {
        //SET_TABLENAME::iterator it = m_vecTableName[nIndex]->find(rsTableName);
        //DB_MemoryCreateBuffer(it != m_vecTableName[nIndex]->end());// ±Ì“—æ≠¥Ê‘⁄£¨ø…“‘∑≈–ƒ π”√
    }
    //KGLOG_PROCESS_ERROR(CreateRedisHashTable(nIndex, rsTableName));
    {
        KTableName sSave;
        sSave.bGlobal = rsTableName.bGlobal;
        char* pName = new char[rsTableName.uLen + 1];
        memcpy(pName, rsTableName.pcszName, rsTableName.uLen);
        pName[rsTableName.uLen] = '\0';
        sSave.pcszName = pName;
        sSave.uLen = rsTableName.uLen;
        m_vecTableName[nIndex]->insert(sSave);
    }
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::RemoveRedisHashTableCache(int nIndex, const KTableName& rsTableName)
{
    SET_TABLENAME::iterator it = m_vecTableName[nIndex]->find(rsTableName);
    if (it != m_vecTableName[nIndex]->end())
    {
        m_vecTableName[nIndex]->erase(it);
    }
    return true;
}

bool KMysqlClient::CheckRedisHastTableIsExist(int nIndex, const KTableName& rsTableName)
{
    bool bResult = false;
    //KGLOG_PROCESS_ERROR(rsTableName.pcszName);
    {
        //SET_TABLENAME::iterator it = m_vecTableName[nIndex]->find(rsTableName);
        //DB_MemoryCreateBuffer(it != m_vecTableName[nIndex]->end());	// ”–≤Ÿ◊˜π˝¥À ±±Ì“ª∂® «¥Ê‘⁄µƒ
    }
    {
        char* pWritePos = m_pSQLBuff;
        int nRetCode = 0;
        if (rsTableName.bGlobal)
        {
            nRetCode = sprintf(pWritePos, "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA=\'%s\' AND TABLE_NAME=\'%s", m_szDBName, GLOBAL_TABLE_NAME_PREFIX);
        }
        else
        {
            nRetCode = sprintf(pWritePos, "SELECT TABLE_NAME FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA=\'%s\' AND TABLE_NAME=\'%s", m_szDBName, HASH_TABLE_NAME_PREFIX);
        }
        pWritePos += nRetCode;
        memcpy(pWritePos, rsTableName.pcszName, rsTableName.uLen);
        pWritePos += rsTableName.uLen;
        *pWritePos++ = '\'';
        nRetCode = (int)(pWritePos - m_pSQLBuff);
        _ASSERT(nRetCode < (int)m_uBuffLen);
        nRetCode = _DoQuery(nIndex, m_pSQLBuff, nRetCode);
        //KGLOG_PROCESS_SUCCESS(!nRetCode);// ÷¥–– ß∞‹“≤»œŒ™’“µΩ±Ì¡À£¨÷Æ∫Û÷¥––µƒ ±∫Úª·±®¥Ì
        {
            KMySQLResult cMysqljResult = _GetQueryResult(nIndex);
            if(!cMysqljResult.IsValid())
                goto Exit0;
            if(cMysqljResult.GetRowCount() != 1)
                goto Exit0;
            {
                KTableName sSave;
                sSave.bGlobal = rsTableName.bGlobal;
                char* pName = new char[rsTableName.uLen + 1];
                memcpy(pName, rsTableName.pcszName, rsTableName.uLen);
                pName[rsTableName.uLen] = '\0';
                sSave.pcszName = pName;
                sSave.uLen = rsTableName.uLen;
                m_vecTableName[nIndex]->insert(sSave);
            }
        }
    }
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::CreateRedisHashTable(int nIndex, const KTableName& rsTableName)
{
    bool bResult = false;
    int nRetCode = 0;
    static unsigned int s_uQueryCreate = sizeof("CREATE TABLE IF NOT EXISTS %s (key_name VARCHAR(333) CHARACTER SET utf8 PRIMARY KEY, data LONGBLOB, modifytime TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP) CHARSET=utf8");
    char szQuery[1024];
    char* pData = szQuery;
    unsigned int uLen = s_uQueryCreate + rsTableName.uLen;
    if((uLen < 1024u))
        goto Exit0;
    nRetCode = sprintf(pData, "CREATE TABLE IF NOT EXISTS ");
    pData += nRetCode;
    if (rsTableName.bGlobal)
    {
        memcpy(pData, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
        pData += GLOBAL_TABLE_NAME_PREFIX_LEN;
    }
    else
    {
        memcpy(pData, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
        pData += HASH_TABLE_NAME_PREFIX_LEN;
    }
    
    memcpy(pData, rsTableName.pcszName, rsTableName.uLen);
    pData += rsTableName.uLen;
    nRetCode = sprintf(pData, " (key_name VARCHAR(333) CHARACTER SET utf8 PRIMARY KEY, data LONGBLOB, modifytime TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP) CHARSET=utf8");
    pData += nRetCode;
    nRetCode = (int)(pData - szQuery);
    nRetCode = _DoQuery(nIndex, szQuery, nRetCode);
    //KGLOG_PROCESS_ERROR(nRetCode);
    bResult = true;
Exit0:
    return bResult;
}

KMySQLResult KMysqlClient::_GetQueryResult(int nIndex, KE_GETRESULT_METHOD eGetMethod /*= emKGET_RESULT_STORE*/)
{
    MYSQL_RES* pRes;
    MYSQL* pMySQL = m_vecMysql[nIndex];
    if (eGetMethod == emKGET_RESULT_STORE)
    {
        pRes = mysql_store_result(pMySQL);
    }
    else
    {
        pRes = mysql_use_result(pMySQL);
    }
    if (!pRes)
    {
        //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] _GetQueryResult err: %s\n", m_szDBName, nIndex, mysql_error(pMySQL));
    }
    return pRes;
}

bool KMysqlClient::_RedisHashTableSet(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen, const char* pcszData, unsigned int uDataLen)
{
    bool bResult = false;
    bool bRet = false;
    int nRetCode = 0;
    unsigned int uSQLLen = 0;
    unsigned int uRet = 0;
    static unsigned int s_uUpdateFixedLen = sizeof("INSERT INTO (key_name,data) values(\'\',\'\') ON DUPLICATE KEY UPDATE data=\'\' ");
    if (pcszData == NULL)
    {
        //bResult = _RedisHashTableDel(nIndex, rsTableName, pcszKey, uKeyLen);
        goto Exit0;
    }
    
    do
    {
        MYSQL* pMySQL = m_vecMysql[nIndex];
        //uSQLLen = s_uUpdateFixedLen + uKeyLen + rsTableName.uLen + GLOBAL_TABLE_NAME_PREFIX_LEN + uDataLen * 4;
        uSQLLen = s_uUpdateFixedLen + uKeyLen + uDataLen * 4;
        if (uSQLLen > m_uBuffLen)
        {
            SAFE_DELETE_ARRAY(m_pSQLBuff);
            m_pSQLBuff = new char[uSQLLen];
            //KGLogPrintf(KGLOG_INFO, "MysqlClient[%s:%d] new buff size[%d]\n", m_szDBName, nIndex, uSQLLen);
            m_uBuffLen = uSQLLen;
        }
        char* pCurWritePos = m_pSQLBuff;
        int nSize = sprintf(pCurWritePos, "INSERT ");
        pCurWritePos += nSize;
        if (rsTableName.bGlobal)
        {
            memcpy(pCurWritePos, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += GLOBAL_TABLE_NAME_PREFIX_LEN;
        }
        else
        {
            memcpy(pCurWritePos, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += HASH_TABLE_NAME_PREFIX_LEN;
        }
        
        memcpy(pCurWritePos, rsTableName.pcszName, rsTableName.uLen);
        pCurWritePos += rsTableName.uLen;
        nSize = sprintf(pCurWritePos, " (key_name,data) values(\'");
        pCurWritePos += nSize;
        uRet = (unsigned int)mysql_real_escape_string(pMySQL, pCurWritePos, pcszKey, uKeyLen);
        pCurWritePos += uRet;
        nSize = sprintf(pCurWritePos, "\',\'");
        pCurWritePos += nSize;
        if (uDataLen > 0)
        {
            uRet = (unsigned int)mysql_real_escape_string(pMySQL, pCurWritePos, pcszData, uDataLen);
            pCurWritePos += uRet;
        }
        nSize = sprintf(pCurWritePos, "\') ON DUPLICATE KEY UPDATE data=\'");
        pCurWritePos += nSize;
        if (uDataLen > 0)
        {
            uRet = (unsigned int)mysql_real_escape_string(pMySQL, pCurWritePos, pcszData, uDataLen);
            pCurWritePos += uRet;
        }
        *pCurWritePos++ = '\'';
        nRetCode = (int)(pCurWritePos - m_pSQLBuff);
        _ASSERT(nRetCode <= (int)uSQLLen);
        bRet = _DoQuery(nIndex, m_pSQLBuff, nRetCode);
        //DB_MemoryCreateBuffer(bRet);
        if (mysql_errno(pMySQL) == ER_NO_SUCH_TABLE)// √ª”–±Ì£¨”–ø…ƒ‹±Ì±ª¡Ì“ª∏ˆ¥˙¿Ì∏¯…æ≥˝¡À£¨∞—±ÌΩ®∆¿¥÷ÿ–¬÷¥––“ª±È
        {
            //KGLOG_PROCESS_ERROR(CreateRedisHashTable(nIndex, rsTableName));
            continue;
        }
        break;
    } while (true);
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::_RedisHashTableDel(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen)
{
    static unsigned int s_uDeleteFixedLen = sizeof("DELETE FROM  where key_name=\'\'");
    bool bResult = false;
    int nRetCode = 0;
    bool bRet = 0;
    unsigned int uSQLLen = 0;
    unsigned int uRet = 0;
    char* pCurWritePos = NULL;
    MYSQL* pMySQL = m_vecMysql[nIndex];
    //KGLOG_PROCESS_ERROR(pcszKey);
    //KGLOG_PROCESS_ERROR(uKeyLen > 0);
    //KGLOG_PROCESS_ERROR(uKeyLen <= KD_MAX_DB_KEY_LEN);
    uSQLLen = s_uDeleteFixedLen + uKeyLen + rsTableName.uLen + GLOBAL_TABLE_NAME_PREFIX_LEN;
    if (uSQLLen > m_uBuffLen)// ª∫≥Â«¯πª¥Û
    {
        SAFE_DELETE_ARRAY(m_pSQLBuff);
        m_pSQLBuff = new char[uSQLLen];
        //KGLogPrintf(KGLOG_INFO, "MysqlClient[%s:%d] new buff size[%d]\n", m_szDBName, nIndex, uSQLLen);
        m_uBuffLen = uSQLLen;
    }
    pCurWritePos = m_pSQLBuff;
    int nSize = sprintf(pCurWritePos, "DELETE FROM ");
    pCurWritePos += nSize;
    if (rsTableName.bGlobal)
    {
        memcpy(pCurWritePos, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
        pCurWritePos += GLOBAL_TABLE_NAME_PREFIX_LEN;
    }
    else
    {
        memcpy(pCurWritePos, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
        pCurWritePos += HASH_TABLE_NAME_PREFIX_LEN;
    }
    
    memcpy(pCurWritePos, rsTableName.pcszName, rsTableName.uLen);
    pCurWritePos += rsTableName.uLen;
    nSize = sprintf(pCurWritePos, " where key_name=\'");
    pCurWritePos += nSize;
    uRet = (unsigned int)mysql_real_escape_string(pMySQL, pCurWritePos, pcszKey, uKeyLen);
    pCurWritePos += uRet;
    *pCurWritePos++ = '\'';
    nRetCode = (int)(pCurWritePos - m_pSQLBuff);
    _ASSERT(nRetCode <= (int)uSQLLen);
    bRet = _DoQuery(nIndex, m_pSQLBuff, nRetCode);
    //DB_MemoryCreateBuffer(bRet);
    //KGLOG_PROCESS_ERROR(mysql_errno(pMySQL) == ER_NO_SUCH_TABLE);// ±Ì∂º≤ª¥Ê‘⁄£¨“ª∂®ƒ‹…æ≥…π¶,µ´ «–Ë“™«Â¿Ì“ªœ¬∂‡”‡µƒ±Ìº«¬º
    RemoveRedisHashTableCache(nIndex, rsTableName);
Exit1:
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::_RedisHashTableGet(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen)
{
    bool bResult = false;
    int nRetCode = 0;
    unsigned int uSQLLen = 0;
    int nTmpSize = 0;
    static unsigned int s_uGetFixedLen = sizeof("SELECT data FROM  WHERE key_name=\'\'");
    MYSQL* pMySQL = m_vecMysql[nIndex];
    char* pCurWritePos = m_pSQLBuff;
    
    uSQLLen = s_uGetFixedLen +uKeyLen + rsTableName.uLen + GLOBAL_TABLE_NAME_PREFIX_LEN;
    _ASSERT(uSQLLen < m_uBuffLen);
    nTmpSize = sprintf(pCurWritePos, "SELECT data FROM ");
    pCurWritePos += nTmpSize;
    if (rsTableName.bGlobal)
    {
        memcpy(pCurWritePos, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
        pCurWritePos += GLOBAL_TABLE_NAME_PREFIX_LEN;
    }
    else
    {
        memcpy(pCurWritePos, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
        pCurWritePos += HASH_TABLE_NAME_PREFIX_LEN;
    }
    memcpy(pCurWritePos, rsTableName.pcszName, rsTableName.uLen);
    pCurWritePos += rsTableName.uLen;
    nTmpSize = sprintf(pCurWritePos, " WHERE key_name=\'");
    pCurWritePos += nTmpSize;
    nTmpSize = (unsigned int)mysql_real_escape_string(pMySQL, pCurWritePos, pcszKey, uKeyLen);
    pCurWritePos += nTmpSize;
    *pCurWritePos++ = '\'';
    nRetCode = (int)(pCurWritePos - m_pSQLBuff);
    _ASSERT(nRetCode <= (int)m_uBuffLen);
    nRetCode = _DoQuery(nIndex, m_pSQLBuff, nRetCode);
    //KGLOG_PROCESS_ERROR(nRetCode);
    bResult = true;
Exit0:
    return bResult;
}

IKG_Buffer* KMysqlClient::GenCommonRespond(long long lId, unsigned char byType, int nRetType, MYSQL* pMysql)
{
    _ASSERT(pMysql);
    IKG_Buffer* pBuffer = NULL;
    if (nRetType == KDB_REPLY_ERROR)
    {
        const char* pcszErr = mysql_error(pMysql);
        size_t uErrLen = strlen(pcszErr);
        pBuffer = DB_MemoryCreateBuffer((unsigned int)(sizeof(KRESOOND_COMMON) + uErrLen));
        KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
        pResond->lId = lId;
        pResond->byHandleFlag = DB_WAIT_FLAG_MYSQL;
        pResond->byRequestType = byType;
        pResond->nRetType = nRetType;
        pResond->nParam = 0;
        pResond->nDataLen = (int)uErrLen;
        memcpy(pResond->data, pcszErr, uErrLen);
        pResond->data[uErrLen] = '\0';
    }
    else
    {
        pBuffer = DB_MemoryCreateBuffer(sizeof(KRESOOND_COMMON) - 1);
        KRESOOND_COMMON* pResond = (KRESOOND_COMMON*)pBuffer->GetData();
        pResond->lId = lId;
        pResond->byHandleFlag = DB_WAIT_FLAG_MYSQL;
        pResond->byRequestType = byType;
        pResond->nRetType = nRetType;
        pResond->nParam = 0;
        pResond->nDataLen = 0;
    }
    char* pData = (char*)pBuffer->GetData();
    pData -= sizeof(unsigned char);
    *(unsigned char*)pData = RESPOND_FLAG_COMMON;
    return pBuffer;
}

bool KMysqlClient::_DoQuery(int nIndex, const char cszQuery[], unsigned int uLength)
{
    bool bResult = false;
    int nRetCode = 0;
    unsigned int uMySQLErrorCode = 0;
    int nReconnectTimes = 0;
    MYSQL* pMysql = m_vecMysql[nIndex];
    //KGLOG_PROCESS_ERROR(cszQuery);
    if (m_bMySQLQueryPrint)
    {
        char szBuf[1024];
        unsigned int uLen = (sizeof(szBuf) - 1) < uLength ? (sizeof(szBuf) - 1) : uLength;
        memcpy(szBuf, cszQuery, uLen);
        szBuf[uLen] = '\0';
        //KGLogPrintf(KGLOG_INFO, "MysqlClient[%s:%d] Query: %s\n", m_szDBName, nIndex, szBuf);
    }
    while (true)
    {
        nRetCode = mysql_real_query(pMysql, cszQuery, uLength);
        if (nRetCode == 0)
            break;// success
        uMySQLErrorCode = mysql_errno(pMysql);
        if (
            (uMySQLErrorCode != CR_SERVER_LOST) &&
            (uMySQLErrorCode != CR_CONN_HOST_ERROR) &&
            (uMySQLErrorCode != CR_CONN_HOST_ERROR) &&
            (uMySQLErrorCode != CR_SERVER_GONE_ERROR)
            )
            break; // false
        
        while (nReconnectTimes <= 10)
        {
            //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s%d] err: Connection lost, try reconnect[%d].\n", m_szDBName, nIndex, nReconnectTimes);
            nRetCode = mysql_ping(pMysql);
            if (nRetCode == 0)
                break;
            ++nReconnectTimes;
        }
        if (nRetCode != 0)
        {
            //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s%d] err: reconnect fail!!!!.\n", m_szDBName, nIndex);
            m_bDatabaseLost = true;
            exit(0);
        }
    }
    bResult = true;
Exit0:
    if (nRetCode != 0)
    {
        bResult = false;
        char szBuf[1024];
        unsigned int uLen = (sizeof(szBuf) - 1) < uLength ? (sizeof(szBuf) - 1) : uLength;
        memcpy(szBuf, cszQuery, uLen);
        szBuf[uLen] = 0;
        //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] Failed MySQL Query: %s\n", m_szDBName, nIndex, szBuf);
        //KGLogPrintf(KGLOG_ERR, "MysqlClient[%s:%d] error: %s\n", m_szDBName, nIndex, mysql_error(pMysql));
    }
    return bResult;
}

bool KMysqlClient::OnRequestSet(int nIndex, const KREQUEST_SET* pRequestSet)
{
    bool bRet = false;
    KTableName sTablename;
    sTablename.bGlobal = true;
    sTablename.pcszName = pRequestSet->data;
    sTablename.uLen = pRequestSet->uPrefixLen;
    const char* pcszKey = pRequestSet->data + pRequestSet->uPrefixLen + REQUEST_KEY_UNDERLINED_LEN;
    const char* pcszData = pRequestSet->data + pRequestSet->uPrefixLen + pRequestSet->uKeyLen + REQUEST_KEY_UNDERLINED_LEN;
    bRet = _RedisHashTableSet(nIndex, sTablename, pcszKey, pRequestSet->uKeyLen, pcszData, pRequestSet->uValueLen);
    _ASSERT(bRet);
    if(!bRet)
    {
        exit(10003);
    }
    
    return bRet;
}

IKG_Buffer* KMysqlClient::OnRequestGet(int nIndex, const KREQUEST_GET* pRequestGet)
{
    bool bExistTable = false;
    IKG_Buffer* pPackatBuffer = NULL;
    KTableName sTablename;
    sTablename.bGlobal = true;
    sTablename.pcszName = pRequestGet->data;
    sTablename.uLen = pRequestGet->uPrefixLen;
    const char* pcszKey = pRequestGet->data + pRequestGet->uPrefixLen + REQUEST_KEY_UNDERLINED_LEN;
    bool bRet = _RedisHashTableGetTest(nIndex, sTablename, pcszKey, pRequestGet->uKeyLen, bExistTable);
    if (bRet)
    {
        pPackatBuffer = _GenGetNetRespond(nIndex, bRet, bExistTable, pRequestGet->lRef, 0, pRequestGet->byType, pRequestGet->nConnectId);
    }
    
    return pPackatBuffer;
}

IKG_Buffer* KMysqlClient::_GenDataNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nRetType, const char* pContent, int nContentLen)
{
    int nProtocolReserved = GetNetPacketReserved();
    IKG_Buffer* pPackatBuffer = DB_MemoryCreateBuffer(sizeof(KRESOOND_COMMON) + nProtocolReserved + nContentLen + 1);
    
    return pPackatBuffer;
}

IKG_Buffer* KMysqlClient::_GenQueryResult(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nRetType, const char* pContent, int nContentLen)
{
    int uSize = sizeof(KRESOOND_COMMON) + nContentLen;
    IKG_Buffer* pPackatBuffer = DB_MemoryCreateBuffer(uSize);
    KRESOOND_COMMON* pResponse = (KRESOOND_COMMON*)pPackatBuffer->GetData();
    pResponse->nDataLen = nContentLen;
    memcpy(pResponse->data, pContent, (size_t)nContentLen);
    
    return pPackatBuffer;
}

IKG_Buffer* KMysqlClient::_GenMultiDataNetRespond(bool bSecondPart, long long lId, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nQueryCount, MYSQL_RES* pMysqlRes[KDB_MAX_SETS_GETS_NUM])
{
    int nProtocolReserved = GetNetPacketReserved();
    IKG_Buffer* pPackatBuffer = DB_MemoryCreateBuffer(sizeof(KRESOOND_COMMON) + nProtocolReserved + 1);
    return pPackatBuffer;
}

IKG_Buffer* KMysqlClient::_GenOKNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId)
{
    int nProtocolReserved = GetNetPacketReserved();
    IKG_Buffer* pPackatBuffer = DB_MemoryCreateBuffer(sizeof(KRESOOND_COMMON) + 3 + nProtocolReserved);
    
    return pPackatBuffer;
}

IKG_Buffer* KMysqlClient::_GenErrorNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, const char* pcszErr, unsigned int uErrLen)
{
    int nProtocolReserved = GetNetPacketReserved();
    IKG_Buffer* pPackatBuffer = DB_MemoryCreateBuffer(sizeof(KRESOOND_COMMON) + 3 + nProtocolReserved);
    
    return pPackatBuffer;
}

bool KMysqlClient::_RedisHashTableGetTest(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen, bool& bExistTable)
{
    MYSQL* pMysql = m_vecMysql[nIndex];
    bool bRet = false;
    if (CheckRedisHastTableIsExist(nIndex, rsTableName))
    {
        bExistTable = true;
        bRet = _RedisHashTableGet(nIndex, rsTableName, pcszKey, uKeyLen);
        if (!bRet)
        {
            unsigned int uLastErrorCode = mysql_errno(pMysql);
            if (uLastErrorCode == ER_NO_SUCH_TABLE)
            {
                bExistTable = false;
                bRet = true;
            }
        }
    }
    else
    {
        bExistTable = false;
        bRet = true;
    }
    return bRet;
}

bool KMysqlClient::_RedisHashTableDelTest(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen, bool& bExistTable)
{
    MYSQL* pMysql = m_vecMysql[nIndex];
    bool bRet = false;
    if (CheckRedisHastTableIsExist(nIndex, rsTableName))
    {
        bExistTable = true;
        bRet = _RedisHashTableDel(nIndex, rsTableName, pcszKey, uKeyLen);
        if (!bRet)
        {
            unsigned int uLastErrorCode = mysql_errno(pMysql);
            if (uLastErrorCode == ER_NO_SUCH_TABLE)// ’“≤ªµΩ±Ì,≤ª–Ë“™±®¥Ì£¨µ±…æ≥˝≥…π¶
            {
                bExistTable = false;
                bRet = true;
            }
        }
    }
    else
    {
        bExistTable = false;
        bRet = true;
    }
    return bRet;
}

bool KMysqlClient::_RedisHashTableGetAll(int nIndex, const KTableName& rsTableName, bool& bExistTable)
{
    MYSQL* pMysql = m_vecMysql[nIndex];
    bool bResult = false;
    if (CheckRedisHastTableIsExist(nIndex, rsTableName))
    {
        bExistTable = true;
        char* pCurWritePos = m_pSQLBuff;
        int nTmpSize = sprintf(pCurWritePos, "SELECT key_name,data FROM ");
        pCurWritePos += nTmpSize;
        if (rsTableName.bGlobal)
        {
            memcpy(pCurWritePos, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += GLOBAL_TABLE_NAME_PREFIX_LEN;
        }
        else
        {
            memcpy(pCurWritePos, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += HASH_TABLE_NAME_PREFIX_LEN;
        }
        memcpy(pCurWritePos, rsTableName.pcszName, rsTableName.uLen);
        pCurWritePos += rsTableName.uLen;
        bool bRetCode = _DoQuery(nIndex, m_pSQLBuff, (unsigned int)(pCurWritePos-m_pSQLBuff));
        if (!bRetCode)
        {
            unsigned int uLastErrorCode = mysql_errno(pMysql);
            if (uLastErrorCode != ER_NO_SUCH_TABLE)// ’“≤ªµΩ±Ì,≤ª–Ë“™±®¥Ì£¨µ±nil∑µªÿ
            {
                //KGLOG_PROCESS_ERROR(false);
            }
            bExistTable = false;
        }
    }
    else
    {
        bExistTable = false;
    }
    bResult = true;
Exit0:
    return bResult;
}

bool KMysqlClient::_RedisHashTableDropTest(int nIndex, const KTableName& rsTableName, bool& bExistTable)
{
    MYSQL* pMysql = m_vecMysql[nIndex];
    bool bResult = false;
    if (CheckRedisHastTableIsExist(nIndex, rsTableName))
    {
        bExistTable = true;
        char* pCurWritePos = m_pSQLBuff;
        int nTmpSize = sprintf(pCurWritePos, "DROP TABLE ");
        pCurWritePos += nTmpSize;
        if (rsTableName.bGlobal)
        {
            memcpy(pCurWritePos, GLOBAL_TABLE_NAME_PREFIX, GLOBAL_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += GLOBAL_TABLE_NAME_PREFIX_LEN;
        }
        else
        {
            memcpy(pCurWritePos, HASH_TABLE_NAME_PREFIX, HASH_TABLE_NAME_PREFIX_LEN);
            pCurWritePos += HASH_TABLE_NAME_PREFIX_LEN;
        }
        memcpy(pCurWritePos, rsTableName.pcszName, rsTableName.uLen);
        pCurWritePos += rsTableName.uLen;
        bool bRetCode = _DoQuery(nIndex, m_pSQLBuff, (int)(pCurWritePos - m_pSQLBuff));
        if (!bRetCode)
        {
            unsigned int uLastErrorCode = mysql_errno(pMysql);
            if (uLastErrorCode != ER_NO_SUCH_TABLE && uLastErrorCode != ER_BAD_TABLE_ERROR)// ’“≤ªµΩ±Ì,≤ª–Ë“™±®¥Ì£¨µ±÷¥––≥…π¶
            {
                //KGLOG_PROCESS_ERROR(false);
            }
            bExistTable = false;
        }
        else
        {
            SET_TABLENAME::iterator it = m_vecTableName[nIndex]->find(rsTableName);
            if (it != m_vecTableName[nIndex]->end())
            {
                delete it->pcszName;
                m_vecTableName[nIndex]->erase(it);
            }
        }
    }
    else
    {
        bExistTable = false;
    }
    bResult = true;
Exit0:
    return bResult;
}

IKG_Buffer* KMysqlClient::_GenGetNetRespond(int nIndex, bool bQueryRet, bool bExistTable, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId)
{
    MYSQL* pMysql = m_vecMysql[nIndex];
    IKG_Buffer* pPackatBuffer = NULL;
    if (!bQueryRet)
    {
        const char* pcszErr = mysql_error(pMysql);
        size_t uErrLen = strlen(pcszErr);
        pPackatBuffer = _GenDataNetRespond(lRef, byProtocolId, byRequestType, nConnectId, KDB_REPLY_ERROR, pcszErr, (int)uErrLen);
        goto Exit1;
    }
    if (!bExistTable)
    {
        pPackatBuffer = _GenDataNetRespond(lRef, byProtocolId, byRequestType, nConnectId, KDB_REPLY_NIL, NULL, 0);
        goto Exit1;
    }
    {
        KMySQLResult cMysqljResult = _GetQueryResult(nIndex);
        if(!cMysqljResult.IsValid())
            goto Exit0;
        if(cMysqljResult.GetFieldCount() != 1)
            goto Exit0;
        if (cMysqljResult.GetRowCount() == 0)
        {
            pPackatBuffer = _GenDataNetRespond(lRef, byProtocolId, byRequestType, nConnectId, KDB_REPLY_NIL, NULL, 0);
            cMysqljResult.Release();
            goto Exit1;
        }
        {
            KMySQLRow cRow = cMysqljResult.GetNextRow();
            unsigned int nQueryDataLen = (unsigned int)cRow.aFieldLenth[0];
            char* pQueryData = cRow.aFieldData[0];
            pPackatBuffer = _GenQueryResult(lRef, byProtocolId, byRequestType, nConnectId, KDB_REPLY_STRING, pQueryData, nQueryDataLen);
        }
        cMysqljResult.Release();
    }
Exit1:
Exit0:
    return pPackatBuffer;
}

bool KMysqlClient::OnRequestHSet(int nIndex, const KREQUEST_HSET* pRequestHSet)
{
    bool bResult = false;
    bool bRet = false;
    
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHSet->data;
    sTablename.uLen = pRequestHSet->uTableNameLen;
    const char* pcszKey = pRequestHSet->data + pRequestHSet->uTableNameLen;
    const char* pcszData = pRequestHSet->data + pRequestHSet->uTableNameLen + pRequestHSet->uHashKeyLen;
    bRet = _RedisHashTableSet(nIndex, sTablename, pcszKey, pRequestHSet->uHashKeyLen, pcszData, pRequestHSet->uValueLen);
    _ASSERT(bRet);
    
    bResult = true;
    return bResult;
}

IKG_Buffer* KMysqlClient::OnRequestHGet(int nIndex, const KREQUEST_HGET* pRequestHGet)
{
    bool bExistTable = false;
    IKG_Buffer* pPackatBuffer = NULL;
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHGet->data;
    sTablename.uLen = pRequestHGet->uTableNameLen;
    const char* pcszKey = pRequestHGet->data + pRequestHGet->uTableNameLen;
    bool bRet = _RedisHashTableGetTest(nIndex, sTablename, pcszKey, pRequestHGet->uHashKeyLen, bExistTable);
    if (bRet)
    {
        pPackatBuffer = _GenGetNetRespond(nIndex, bRet, bExistTable, pRequestHGet->lRef, 0, pRequestHGet->byType, pRequestHGet->nConnectId);
    }
    
    return pPackatBuffer;
}

bool KMysqlClient::OnRequestSets(int nIndex, const KREQUEST_SETS* pRequestSets)
{
    bool bResult = false;
    bool bRet = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_VALUE_DESC* pKVDesc = (const KREQUEST_KEY_VALUE_DESC*)pRequestSets->data;
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    
    for (int i = 0; i < pRequestSets->nCount; i++)
    {
        sTablename.bGlobal = true;
        sTablename.pcszName = pKVDesc[i].pKey;
        sTablename.uLen = pKVDesc[i].uKeyLen - pRequestSets->uHashKeyLen - 1;
        const char* pcszKey = pKVDesc[i].pKey + pKVDesc[i].uKeyLen - pRequestSets->uHashKeyLen;
        bRet = _RedisHashTableSet(nIndex, sTablename, pcszKey, pRequestSets->uHashKeyLen, pKVDesc[i].pValue, pKVDesc[i].uValueLen);
        _ASSERT(bRet);
        if (!bRet)
        {
            const char* pcszErr = mysql_error(pMysql);
            if (pcszErr)
            {
                //KGLogPrintf(KGLOG_ERR, "KMysqlClient::OnRequestSets set table_key[%s]error:%s\n", pKVDesc->pKey, pcszErr);
            }
            else
            {
                //KGLogPrintf(KGLOG_ERR, "KMysqlClient::OnRequestSets set table_key[%s] error:unkonw\n", pKVDesc->pKey, pKVDesc->pKey);
            }
        }
    }
    bRet = true;
    if (pRequestSets->byHandleFlag & DB_FLAG_MYSQL)
    {
       // pPackatBuffer = _GenOKNetRespond(pRequestSets->lRef, PROXY2DRIVE_DATA_SETS, pRequestSets->byType, pRequestSets->nConnectId);
        PushRespond(pPackatBuffer);
    }
    if (bRet)
    {
        pCommonBuffer = GenCommonRespond(pRequestSets->lId, pRequestSets->byType, KDB_REPLY_STRING, pMysql);
    }
    else
    {
        pCommonBuffer = GenCommonRespond(pRequestSets->lId, pRequestSets->byType, KDB_REPLY_ERROR, pMysql);
    }
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestGets(int nIndex, const KREQUEST_GETS* pRequestGets)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_DESC* pKeyDesc = (const KREQUEST_KEY_DESC*)pRequestGets->data;
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    MYSQL_RES* pMysqlRes[KDB_MAX_SETS_GETS_NUM];
    memset(pMysqlRes, 0, sizeof(pMysqlRes));
    for (int i = 0; i < pRequestGets->nCount; i++)
    {
        if (!pKeyDesc->bIsNil)
        {
            ++pKeyDesc;
            continue;
        }
        
        sTablename.bGlobal = true;
        sTablename.pcszName = pKeyDesc->pKey;
        sTablename.uLen = pKeyDesc->uKeyLen - pRequestGets->uHashKeyLen - 1;
        const char* pcszKey = pKeyDesc->pKey + pKeyDesc->uKeyLen - pRequestGets->uHashKeyLen;
        bool bExistTable = false;
        bool bRet = _RedisHashTableGetTest(nIndex, sTablename, pcszKey, pRequestGets->uHashKeyLen, bExistTable);
        if (bExistTable && bRet)	// √ª’“µΩ÷µµƒ≤ª–Ë“™¥¶¿Ì
        {
            pMysqlRes[i] = mysql_store_result(pMysql);
            _ASSERT(pMysqlRes[i]);
        }
        ++pKeyDesc;
    }
    if (pRequestGets->byHandleFlag & DB_FLAG_MYSQL)
    {
        if (pRequestGets->byHandleFlag & DB_FLAG_REDIS)	// redis“≤¥¶¿Ìµƒ£¨Àµ√˜mysql÷ª «±∏”√≤π»´
        {
            //pPackatBuffer = _GenMultiDataNetRespond(true, pRequestGets->lId, pRequestGets->lRef, PROXY2DRIVE_DATA_GETS, pRequestGets->byType, pRequestGets->nConnectId, pRequestGets->nCount, pMysqlRes);
        }
        else
        {
            //pPackatBuffer = _GenMultiDataNetRespond(false, pRequestGets->lId, pRequestGets->lRef, PROXY2DRIVE_DATA_GETS, pRequestGets->byType, pRequestGets->nConnectId, pRequestGets->nCount, pMysqlRes);
        }
        
        PushRespond(pPackatBuffer);
    }
    for (int i = 0; i < pRequestGets->nCount; i++)
    {
        if (pMysqlRes[i])
            mysql_free_result(pMysqlRes[i]);
    }
    pCommonBuffer = GenCommonRespond(pRequestGets->lId, pRequestGets->byType, KDB_REPLY_STRING, pMysql);
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestHSets(int nIndex, const KREQUEST_HSETS* pRequestHSets)
{
    bool bResult = false;
    bool bRet = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_VALUE_DESC* pKVDesc = (const KREQUEST_KEY_VALUE_DESC*)(pRequestHSets->data + pRequestHSets->uTableNameLen + 1);
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHSets->data;
    sTablename.uLen = pRequestHSets->uTableNameLen;
    for (int i = 0; i < pRequestHSets->nCount; i++)
    {
        bRet = _RedisHashTableSet(nIndex, sTablename, pKVDesc->pKey, pKVDesc->uKeyLen, pKVDesc->pValue, pKVDesc->uValueLen);
        _ASSERT(bRet);
        if (!bRet)
        {
            const char* pcszErr = mysql_error(pMysql);
            if (pcszErr)
            {
                //KGLogPrintf(KGLOG_ERR, "KMysqlClient::OnRequestHSets set table[%s] key[%s] error:%s\n", pRequestHSets->data, pKVDesc->pKey, pcszErr);
            }
            else
            {
                //KGLogPrintf(KGLOG_ERR, "KMysqlClient::OnRequestHSets set table[%s] key[%s] error:unkonw\n", pRequestHSets->data, pKVDesc->pKey);
            }
        }
        pKVDesc++;
    }
    bRet = true;
    if (pRequestHSets->byHandleFlag & DB_FLAG_MYSQL)
    {
        //pPackatBuffer = _GenOKNetRespond(pRequestHSets->lRef, PROXY2DRIVE_DATA_HSETS, pRequestHSets->byType, pRequestHSets->nConnectId);
        PushRespond(pPackatBuffer);
    }
    if (bRet)
    {
        pCommonBuffer = GenCommonRespond(pRequestHSets->lId, pRequestHSets->byType, KDB_REPLY_STRING, pMysql);
    }
    else
    {
        pCommonBuffer = GenCommonRespond(pRequestHSets->lId, pRequestHSets->byType, KDB_REPLY_ERROR, pMysql);
    }
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestHGets(int nIndex, const KREQUEST_HGETS* pRequestHGets)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_DESC* pKeyDesc = (const KREQUEST_KEY_DESC*)(pRequestHGets->data + pRequestHGets->uTableNameLen + 1);
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHGets->data;
    sTablename.uLen = pRequestHGets->uTableNameLen;
    MYSQL_RES* pMysqlRes[KDB_MAX_SETS_GETS_NUM];
    memset(pMysqlRes, 0, sizeof(pMysqlRes));
    for (int i = 0; i < pRequestHGets->nCount; i++)
    {
        bool bExistTable = false;
        bool bRet = _RedisHashTableGetTest(nIndex, sTablename, pKeyDesc->pKey, pKeyDesc->uKeyLen, bExistTable);
        if (!bExistTable)	// ±Ì≤ª¥Ê‘⁄£¨øœ∂®√ª÷µ
            break;
        if (bRet)	// √ª’“µΩ÷µµƒ≤ª–Ë“™¥¶¿Ì
        {
            pMysqlRes[i] = mysql_store_result(pMysql);
            _ASSERT(pMysqlRes[i]);
        }
        ++pKeyDesc;
    }
    if (pRequestHGets->byHandleFlag & DB_FLAG_MYSQL)
    {
        //pPackatBuffer = _GenMultiDataNetRespond(false, pRequestHGets->lId, pRequestHGets->lRef, PROXY2DRIVE_DATA_HGETS, pRequestHGets->byType, pRequestHGets->nConnectId, pRequestHGets->nCount, pMysqlRes);
        PushRespond(pPackatBuffer);
    }
    for (int i = 0; i < pRequestHGets->nCount; i++)
    {
        if (pMysqlRes[i])
            mysql_free_result(pMysqlRes[i]);
    }
    pCommonBuffer = GenCommonRespond(pRequestHGets->lId, pRequestHGets->byType, KDB_REPLY_STRING, pMysql);
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestHGetAll(int nIndex, const KREQUEST_HGETALL* pRequestHGetAll)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHGetAll->data;
    sTablename.uLen = pRequestHGetAll->uTableNameLen;
    bool bExistTable = false;
    bool bRet = _RedisHashTableGetAll(nIndex, sTablename, bExistTable);
    if (pRequestHGetAll->byHandleFlag & DB_FLAG_MYSQL)
    {
        if (bRet)
        {
            MYSQL_RES* pMysqlRes = NULL;
            if (bExistTable)
            {
                pMysqlRes = mysql_store_result(pMysql);
            }
            //pPackatBuffer = _GenHGetAllRespond(bExistTable, pRequestHGetAll->lRef, PROXY2DRIVE_DATA_HGETALL, pRequestHGetAll->byType, pRequestHGetAll->nConnectId, pMysqlRes);
            if (pMysqlRes)
            {
                mysql_free_result(pMysqlRes);
            }
        }
        else
        {
            //const char* pcszErr = mysql_error(pMysql);
            //size_t uErrLen = strlen(pcszErr);
            //pPackatBuffer = _GenErrorNetRespond(pRequestHGetAll->lRef, PROXY2DRIVE_DATA_HGETALL, pRequestHGetAll->byType, pRequestHGetAll->nConnectId, pcszErr, uErrLen);
        }
        PushRespond(pPackatBuffer);
    }
    
    pCommonBuffer = GenCommonRespond(pRequestHGetAll->lId, pRequestHGetAll->byType, KDB_REPLY_STRING, pMysql);
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestDel(int nIndex, const KREQUEST_DEL* pRequestDel)
{
    bool bExistTable = false;
    KTableName sTablename;
    sTablename.bGlobal = true;
    sTablename.pcszName = pRequestDel->data;
    sTablename.uLen = pRequestDel->uPrefixLen;
    const char* pcszKey = pRequestDel->data + pRequestDel->uPrefixLen + REQUEST_KEY_UNDERLINED_LEN;
    bool bRet = _RedisHashTableDelTest(nIndex, sTablename, pcszKey, pRequestDel->uKeyLen, bExistTable);
    if(!bRet)
    {
        exit(10001);
    }
    return bRet;
}

bool KMysqlClient::OnRequestDels(int nIndex, const KREQUEST_DELS* pRequestDels)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_DESC* pKeyDesc = (const KREQUEST_KEY_DESC*)pRequestDels->data;
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    for (int i = 0; i < pRequestDels->nCount; i++)
    {
        sTablename.bGlobal = true;
        sTablename.pcszName = pKeyDesc->pKey;
        sTablename.uLen = pKeyDesc->uKeyLen - pRequestDels->uHashKeyLen - 1;
        const char* pcszKey = pKeyDesc->pKey + pKeyDesc->uKeyLen - pRequestDels->uHashKeyLen;
        bool bExistTable = false;
        bool bRet = _RedisHashTableDelTest(nIndex, sTablename, pcszKey, pRequestDels->uHashKeyLen, bExistTable);
        _ASSERT(bRet);
        ++pKeyDesc;
    }
    if (pRequestDels->byHandleFlag & DB_FLAG_MYSQL)
    {
        //pPackatBuffer = _GenOKNetRespond(pRequestDels->lRef, PROXY2DRIVE_DATA_DELS, pRequestDels->byType, pRequestDels->nConnectId);
        PushRespond(pPackatBuffer);
    }
    
    pCommonBuffer = GenCommonRespond(pRequestDels->lId, pRequestDels->byType, KDB_REPLY_INTEGER, pMysql);
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestHDel(int nIndex, const KREQUEST_HDEL* pRequestHDel)
{
    bool bExistTable = false;
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHDel->data;
    sTablename.uLen = pRequestHDel->uTableNameLen;
    const char* pcszKey = pRequestHDel->data + pRequestHDel->uTableNameLen;
    bool bRet = _RedisHashTableDelTest(nIndex, sTablename, pcszKey, pRequestHDel->uHashKeyLen, bExistTable);
    if (!bRet)
    {
        exit(10002);
    }
    return bRet;
}

bool KMysqlClient::OnRequestHDels(int nIndex, const KREQUEST_HDELS* pRequestHDels)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    const KREQUEST_KEY_DESC* pKeyDesc = (const KREQUEST_KEY_DESC*)(pRequestHDels->data + pRequestHDels->uTableNameLen + 1);
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestHDels->data;
    sTablename.uLen = pRequestHDels->uTableNameLen;
    for (int i = 0; i < pRequestHDels->nCount; i++)
    {
        bool bExistTable = false;
        bool bRet = _RedisHashTableDelTest(nIndex, sTablename, pKeyDesc->pKey, pKeyDesc->uKeyLen, bExistTable);
        _ASSERT(bRet);
        ++pKeyDesc;
    }
    if (pRequestHDels->byHandleFlag & DB_FLAG_MYSQL)
    {
        //pPackatBuffer = _GenOKNetRespond(pRequestHDels->lRef, PROXY2DRIVE_DATA_HDELS, pRequestHDels->byType, pRequestHDels->nConnectId);
        PushRespond(pPackatBuffer);
    }
    pCommonBuffer = GenCommonRespond(pRequestHDels->lId, pRequestHDels->byType, KDB_REPLY_INTEGER, pMysql);
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}

bool KMysqlClient::OnRequestDelHashTable(int nIndex, const KREQUEST_DELHASHTABLE* pRequestDelHashTable)
{
    bool bResult = false;
    IKG_Buffer* pPackatBuffer = NULL;
    IKG_Buffer* pCommonBuffer = NULL;
    MYSQL* pMysql = m_vecMysql[nIndex];
    KTableName sTablename;
    sTablename.bGlobal = false;
    sTablename.pcszName = pRequestDelHashTable->data;
    sTablename.uLen = pRequestDelHashTable->uTableNameLen;
    bool bExistTable = false;
    bool bRet = _RedisHashTableDropTest(nIndex, sTablename, bExistTable);
    if (pRequestDelHashTable->byHandleFlag & DB_FLAG_MYSQL)
    {
        if (bRet)
        {
           // pPackatBuffer = _GenOKNetRespond(pRequestDelHashTable->lRef, PROXY2DRIVE_DATA_DELTB, pRequestDelHashTable->byType, pRequestDelHashTable->nConnectId);
        }
        else
        {
            //const char* pcszErr = mysql_error(pMysql);
            //size_t uErrLen = strlen(pcszErr);
            //pPackatBuffer = _GenErrorNetRespond(pRequestDelHashTable->lRef, PROXY2DRIVE_DATA_DELTB, pRequestDelHashTable->byType, pRequestDelHashTable->nConnectId, pcszErr, uErrLen);
        }
        PushRespond(pPackatBuffer);
    }
    
    if (bRet)
    {
        pCommonBuffer = GenCommonRespond(pRequestDelHashTable->lId, pRequestDelHashTable->byType, KDB_REPLY_INTEGER, pMysql);
    }
    else
    {
        pCommonBuffer = GenCommonRespond(pRequestDelHashTable->lId, pRequestDelHashTable->byType, KDB_REPLY_ERROR, pMysql);
    }
    
    PushRespond(pCommonBuffer);
    bResult = true;
    //Exit0:
    SAFE_RELEASE(pPackatBuffer);
    SAFE_RELEASE(pCommonBuffer);
    return bResult;
}
