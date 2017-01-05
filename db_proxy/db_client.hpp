//
//  db_client.hpp
//  thread
//
//  Created by 冯文斌 on 16/12/7.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef db_client_hpp
#define db_client_hpp

#include <list>
#include <vector>
#include <pthread.h>
#include "db_def.h"
#include "db_buffer.h"

enum KE_DB_TYPE
{
    emDB_TYPE_REDIS,
    emDB_TYPE_MYSQL,
    emDB_TYPE_NUM
};

class KDBClient
{
public:
    KDBClient();
    virtual ~KDBClient();
    
    bool Init(bool bMysql,
              const char* pcszDBAddress,
              int nPort,
              const char* pcszDBName,
              const char* pcszUserName,
              const char* pcszPassWord
              );
    
    bool Init(bool bMysql, const char* pcszDBName, const std::vector<KConnectInfo>& vecConnectInfos);
    virtual bool UnInit();
    
    virtual bool PushRequest(IKG_Buffer* pBuffer);
    virtual bool PushRespond(IKG_Buffer* pBuffer);
    virtual void Activate();
    virtual bool IsEnabled() = 0;
    bool IsOpen() { return !m_bDatabaseLost; }
protected:
    
    void SetMySQLFlag(bool bMySQL);
    bool GetMySQLFlag();
    void SetRunFlag(bool bRun);
    bool GetRunFlag();
    
    virtual bool ConnectDB() = 0;
    virtual void PreThreadActive() = 0;
    virtual bool OnRequest(IKG_Buffer* pBuffer) = 0;
    virtual bool OnResponsed(IKG_Buffer* pBuffer) = 0;
    virtual void Ping() = 0;
    virtual bool Reconnect(int nIndex) = 0;
    
    int GetNetPacketReserved();
    bool SetNetPacketReserved(char* pData);
    
    char m_szDBName[KD_MAX_DBNAME_LEN];
    std::vector<KConnectInfo> m_vecConnectInfo;
    bool m_bDatabaseLost;
    unsigned m_uSleepTime;
    unsigned long m_uPingTime;
    bool m_bMysql;

private:
    static void* WorkThreadFunction(void *pvParam);
    void ThreadFunction();
    void DBThreadActivate();
    
    IKG_Buffer* PopRequest();
    IKG_Buffer* PopRespond();
    
    typedef std::list<IKG_Buffer*> KPACKAGE_QUEUE;
    
    bool m_bRunFlag;
    pthread_t m_threadId;
    pthread_mutex_t m_mutex;
    KPACKAGE_QUEUE          m_RequestQueue;
    KPACKAGE_QUEUE          m_RespondQueue;
};

#endif /* db_client_hpp */
