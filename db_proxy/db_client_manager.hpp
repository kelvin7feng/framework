//
//  db_client_manager.hpppp
//  thread
//
//  Created by 冯文斌 on 16/12/12.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef db_client_manager_hpp
#define db_client_manager_hpp

#include <vector>
#include "kmacros.h"
#include "db_buffer.h"
#include "krequest_def.h"

class KRedisClient;
class KMysqlClient;
class KRequestProcessor;
class KDBClientMgr
{
public:
    KDBClientMgr();
    ~KDBClientMgr();
    
    bool Init();
    bool UnInit();
    void Activate();
    bool ProcessDataProtocol(char *pData, int nSize, int nConnectID);
    bool ProcessRespond(IKG_Buffer* pBuffer);
    bool CheckMysqlEnable(int nDBType);
    bool CheckRedisEnable(int nDBType);
    bool PushRedisRequest(int nDBType, IKG_Buffer* pBuffer);
    bool PushMysqlRequest(int nDBType, IKG_Buffer* pBuffer);
    IKG_Buffer* RequestMySQLQuery(int nDBType, IKG_Buffer* pBuffer);
    bool PushBothRequest(int nDBType, IKG_Buffer* pBuffer);     //保证同时成功
    int GetNetPacketReserved();
    bool SetNetPacketReserved(char* pDataStart);                //真实数据起始位置
    
protected:
    bool Release();
    
private:
    
    std::vector<KRedisClient*> m_vecRedisClients;
    std::vector<KMysqlClient*> m_vecMysqlClients;
    KRequestProcessor*	m_arrProcessor[emREQUEST_NUM];
};

extern KDBClientMgr* g_pDBClientMgr;

#endif /* db_client_manager_hpp */
