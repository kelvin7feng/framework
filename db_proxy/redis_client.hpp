//
//  redis_client.hpp
//  thread
//
//  Created by 冯文斌 on 16/12/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef redis_client_hpp
#define redis_client_hpp

#include "hiredis.h"
#include "db_client.hpp"
#include "ktimer.h"
#include "krequest_def.h"

class KRedisClient : public KDBClient
{
public:
    KRedisClient();
    virtual ~KRedisClient();
    
    bool Init(int nDBType);
    virtual bool UnInit();
    virtual bool IsEnabled()	{ return m_vecRedisContext.size() > 0; }

protected:
    virtual bool ConnectDB();
    virtual bool Reconnect(int nIndex);
    virtual void PreThreadActive() {};
    virtual bool OnRequest(IKG_Buffer* pBuffer);
    virtual bool OnResponsed(IKG_Buffer* pBuffer);
    virtual void Ping() {}
    
private:
    
    redisReply* RedisCommand(int nIndex, const char* format, ...);
    IKG_Buffer* GenCommonRespond(long long lId, unsigned char byType, redisReply* pReply);
    
    bool OnRequestSet(int nIndex, const KREQUEST_SET* pRequestSet);
    bool OnRequestGet(int nIndex, const KREQUEST_GET* pRequestGet);
    bool OnRequestExpire(int nIndex, const KREQUEST_EXPIRE* pRequestExpire);
    bool OnRequestHSet(int nIndex, const KREQUEST_HSET* pRequestHSet);
    bool OnRequestHGet(int nIndex, const KREQUEST_HGET* pRequestHGet);
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
    
    IKG_Buffer* _GenDataNetRespond(long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, redisReply* pReply);
    IKG_Buffer* _GenMultiDataNetRespond(bool bAllowNil, long long lId, long long lRef, unsigned char byProtocolId, unsigned char byRequestType, int nConnectId, redisReply* pReply);
    
    std::vector<redisContext*> m_vecRedisContext;
    KTimer m_cTimer;
    int m_nQueryCount;
    unsigned long long m_ulAverageUSec;
    
    int m_nRedisMaxReconnectTime = 1;
    int m_nRedisReconnectSleepTime = 1;
    int m_nRedisLogFrequency = 1;
    
};
#endif /* redis_client_hpp */
