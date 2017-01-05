//
//  mysql_client.hpp
//  thread
//
//  Created by 冯文斌 on 16/12/30.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef mysql_client_hpp
#define mysql_client_hpp

#include "krequest_def.h"
#include "db_client.hpp"
#include "mysql_database.h"
#include <mysql.h>
#include <set>
#include <map>

class KMysqlClient : public KDBClient
{
public:
    struct KTableName
    {
        bool bGlobal;
        const char* pcszName;
        unsigned int uLen;
    };
    struct CmpKey : public std::binary_function <const KTableName&, const KTableName&, bool >
    {
        bool operator()(const KTableName& sName1, const KTableName& sName2) const
        {
            if (sName1.bGlobal != sName2.bGlobal)
                return sName1.bGlobal;
            if (sName1.uLen != sName2.uLen)
                return sName1.uLen < sName2.uLen;
            return memcmp(sName1.pcszName, sName2.pcszName, sName1.uLen) < 0;
        }
    };
public:
    KMysqlClient();
    virtual ~KMysqlClient();
    
    bool Init(int nDBType);
    virtual bool UnInit();
    virtual bool IsEnabled()	{ return m_vecMysql.size() > 0; }
public:
    bool CreateDatabase(int nIndex,const char cszDatabaseName[KD_MAX_DBNAME_LEN],int nCreateFlag);
    bool SetCurrentDatabase(MYSQL* pMysql,const char cszDatabaseName[KD_MAX_DBNAME_LEN]);
    IKG_Buffer* OnRequestQuery(IKG_Buffer* pBuffer);
protected:
    virtual bool ConnectDB();
    virtual bool Reconnect(int nIndex);
    virtual void PreThreadActive() {};
    virtual bool OnRequest(IKG_Buffer* pBuffer);
    virtual bool OnResponsed(IKG_Buffer* pBuffer);
    virtual void Ping();
private:
    bool CheckAndCreateRedisHashTable(int nIndex, const KTableName& rsTableName);
    bool RemoveRedisHashTableCache(int nIndex, const KTableName& rsTableName);
    bool CheckRedisHastTableIsExist(int nIndex, const KTableName& rsTableName);
    bool CreateRedisHashTable(int nIndex, const KTableName& rsTableName);
    ////////////////////////////////////////////////////////////////////////////
    // @brief : Get Query Result for Last DoQuery.
    // @return :
    // @remark :KGC_MySQLResult
    KMySQLResult _GetQueryResult(int nIndex, KE_GETRESULT_METHOD eGetMethod = emKGET_RESULT_STORE);
    
    bool _RedisHashTableSet(
                            int nIndex,
                            const KTableName& rsTableName,
                            const char* pcszKey,
                            unsigned int uKeyLen,
                            const char* pcszData,
                            unsigned int uDataLen);
    bool _RedisHashTableDel(
                            int nIndex,
                            const KTableName& rsTableName,
                            const char* pcszKey,
                            unsigned int uKeyLen);
    // redis get, hget
    bool _RedisHashTableGet(
                            int nIndex,
                            const KTableName& rsTableName,
                            const char* pcszKey,
                            unsigned int uKeyLen
                            );
    
    IKG_Buffer* GenCommonRespond(long long lId, unsigned char byType, int nRetType, MYSQL* pMysql);
    
    ////////////////////////////////////////////////////////////////////////////
    // @brief : send query to server, if connect lost, reconnect.
    // @return : true if successful, false if failed.
    bool _DoQuery(int nIndex, const char cszQuery[], unsigned int uLength);
    
    bool OnRequestSet(int nIndex, const KREQUEST_SET* pRequestSet);
    bool OnRequestHSet(int nIndex, const KREQUEST_HSET* pRequestHSet);
    bool OnRequestSets(int nIndex, const KREQUEST_SETS* pRequestSets);
    bool OnRequestGets(int nIndex, const KREQUEST_GETS* pRequestGets);
    bool OnRequestHSets(int nIndex, const KREQUEST_HSETS* pRequestHSets);
    bool OnRequestHGets(int nIndex, const KREQUEST_HGETS* pRequestHGets);
    bool OnRequestHGetAll(int nIndex, const KREQUEST_HGETALL* pRequestHGetAll);
    bool OnRequestDel(int nIndex, const KREQUEST_DEL* pRequestDel);
    bool OnRequestDels(int nIndex, const KREQUEST_DELS* pRequestDels);
    bool OnRequestHDel(int nIndex, const KREQUEST_HDEL* pRequestHDel);
    bool OnRequestHDels(int nIndex, const KREQUEST_HDELS* pRequestHDels);
    bool OnRequestDelHashTable(int nIndex, const KREQUEST_DELHASHTABLE* pRequestDelHashTable);
    
    IKG_Buffer* OnRequestGet(int nIndex, const KREQUEST_GET* pRequestGet);
    IKG_Buffer* OnRequestHGet(int nIndex, const KREQUEST_HGET* pRequestHGet);
private:
    IKG_Buffer* _GenQueryResult(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nRetType, const char* pContent, int nContentLen);
    
    IKG_Buffer* _GenDataNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nRetType, const char* pContent, int nContentLen);
    IKG_Buffer* _GenMultiDataNetRespond(bool bSecondPart, long long lId, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, int nQueryCount, MYSQL_RES* pMysqlRes[KDB_MAX_SETS_GETS_NUM]);
    IKG_Buffer* _GenOKNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId);
    IKG_Buffer* _GenErrorNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, const char* pcszErr, unsigned int uErrLen);
    bool _RedisHashTableGetTest(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen, bool& bExistTable);
    bool _RedisHashTableDelTest(int nIndex, const KTableName& rsTableName, const char* pcszKey, unsigned int uKeyLen, bool& bExistTable);
    bool _RedisHashTableGetAll(int nIndex, const KTableName& rsTableName, bool& bExistTable);
    bool _RedisHashTableDropTest(int nIndex, const KTableName& rsTableName, bool& bExistTable);
    IKG_Buffer* _GenGetNetRespond(int nIndex, bool bQueryRet, bool bExistTable, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId);
    IKG_Buffer* _GenHGetAllRespond(bool bExistTable, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, MYSQL_RES* pMysqlRes);
private:
    typedef std::set<KTableName, CmpKey>	SET_TABLENAME;
    
    std::vector<SET_TABLENAME*> m_vecTableName;
    std::vector<MYSQL*> m_vecMysql;
    char* m_pSQLBuff;
    unsigned int m_uBuffLen;
    
    int m_nMysqlReconnectSleepTime;
    int m_nMysqlMaxReconnectTime;
    bool m_bMySQLQueryPrint;
};
#endif /* mysql_client_hpp */
