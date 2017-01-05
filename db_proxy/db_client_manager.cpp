//
//  db_client_manager.cpp
//  thread
//
//  Created by 冯文斌 on 16/12/12.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <iostream>
#include "db_client_manager.hpp"
#include "redis_client.hpp"
#include "mysql_client.hpp"

KDBClientMgr* g_pDBClientMgr = NULL;

KDBClientMgr::KDBClientMgr()
{
    Init();
}


KDBClientMgr::~KDBClientMgr()
{

}

bool KDBClientMgr::Init()
{
    bool bResult = false;
    for (int i = 1; i <= 1; i++)
    {
        KRedisClient* pRedisClient = new KRedisClient;
        bool bRet = pRedisClient->Init(i);
        if (!bRet)
        {
            exit(1);
        }
        
        m_vecRedisClients.push_back(pRedisClient);
        
    }
    
    for (int i = 1; i <= 1; i++)
    {
        KMysqlClient* pMySQLClient = new KMysqlClient;
        bool bRet = pMySQLClient->Init(i);
        if (!bRet)
        {
            exit(1);
        }
        
        m_vecMysqlClients.push_back(pMySQLClient);
        
    }
    bResult = true;
Exit0:
    if (!bResult)
    {
        Release();
    }
    return bResult;
}

bool KDBClientMgr::UnInit()
{
    return Release();
}

void KDBClientMgr::Activate()
{
    for (std::vector<KRedisClient*>::iterator it = m_vecRedisClients.begin(); it != m_vecRedisClients.end(); ++it)
    {
        if ((*it)->IsEnabled())
        {
            KRedisClient* client = *it;
            client->Activate();
        }
    }
    
    for (std::vector<KMysqlClient*>::iterator it = m_vecMysqlClients.begin(); it != m_vecMysqlClients.end(); ++it)
    {
        if ((*it)->IsEnabled())
        {
            (*it)->Activate();
        }
    }
}

bool KDBClientMgr::CheckRedisEnable(int nDBType)
{
    bool bResult = false;
    bResult = m_vecRedisClients[nDBType - 1]->IsEnabled();
Exit0:
    return bResult;
}

bool KDBClientMgr::PushRedisRequest(int nDBType, IKG_Buffer* pBuffer)
{
    bool bResult = false;
    KRedisClient* redis_client = m_vecRedisClients[nDBType - 1];
    redis_client->PushRequest(pBuffer);
    
    return bResult;
}

bool KDBClientMgr::PushMysqlRequest(int nDBType, IKG_Buffer* pBuffer)
{
    bool bResult = false;
    
    if(nDBType <= 0)
        exit(1);
    //if(m_vecMysqlClients[nDBType - 1]->IsEnabled())
        //exit(1);
    
    std::cout << "push mysql request......." << std::endl;
    m_vecMysqlClients[nDBType - 1]->PushRequest(pBuffer);
    bResult = true;
Exit0:
    return bResult;
}

IKG_Buffer* KDBClientMgr::RequestMySQLQuery(int nDBType, IKG_Buffer* pBuffer)
{
    
    IKG_Buffer* pPackBuffer = NULL;
    if(nDBType <= 0)
        exit(1);
    
    pPackBuffer = m_vecMysqlClients[nDBType - 1]->OnRequestQuery(pBuffer);
    KRESOOND_COMMON* pResponse = (KRESOOND_COMMON*)pPackBuffer->GetData();
    
    return pPackBuffer;
}

bool KDBClientMgr::Release()
{
    for (std::vector<KRedisClient*>::iterator it = m_vecRedisClients.begin(); it != m_vecRedisClients.end(); ++it)
    {
        (*it)->UnInit();
        delete (*it);
    }
    m_vecRedisClients.clear();
    
    for (std::vector<KMysqlClient*>::iterator it = m_vecMysqlClients.begin(); it != m_vecMysqlClients.end(); ++it)
    {
        (*it)->UnInit();
        delete (*it);
    }
    m_vecMysqlClients.clear();
    
    return true;
}