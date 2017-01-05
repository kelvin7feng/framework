//
//  db_client.cpp
//  thread
//
//  Created by 冯文斌 on 16/12/7.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <assert.h>
#include <iostream>
#include "kmacros.h"
#include "db_client.hpp"

KDBClient::KDBClient()
{
    m_bDatabaseLost = true;
    m_bRunFlag = false;
    m_uSleepTime = 5;
    m_uPingTime = 0;
    m_bMysql = false;
    pthread_mutex_init(&m_mutex, NULL);
}

KDBClient::~KDBClient()
{
    m_bRunFlag = false;
    pthread_mutex_destroy(&m_mutex);
}

void KDBClient::SetMySQLFlag(bool bMySQL)
{
    m_bMysql = bMySQL;
}

bool KDBClient::GetMySQLFlag()
{
    return m_bMysql;
}

void KDBClient::SetRunFlag(bool bRun)
{
    m_bRunFlag = bRun;
}

bool KDBClient::GetRunFlag()
{
    return m_bRunFlag;
}

bool KDBClient::Init(bool bMySQL, const char* pcszDBAddress, int nPort, const char* pcszDBName, const char* pcszUserName, const char* pcszPassWord)
{
    bool bResult = false;
    bool bRet = false;
    int nErr = 0;
    
    KConnectInfo sConnectInfo;
    SetMySQLFlag(bMySQL);
    
    strcpy(m_szDBName, pcszDBName);
    strcpy(sConnectInfo.szHost, pcszDBAddress);
    sConnectInfo.nPort = nPort;
    
    if(pcszUserName)
    {
        strcpy(sConnectInfo.szUser, pcszUserName);
    }
    
    if(pcszPassWord)
    {
        strcpy(sConnectInfo.szPwd, pcszPassWord);
    }
    
    m_vecConnectInfo.push_back(sConnectInfo);
    bRet = ConnectDB();
    if(!bRet)
    {
        goto Exit0;
    }
    
    SetRunFlag(true);
    nErr = pthread_create(&m_threadId, NULL, WorkThreadFunction, this);
    if(nErr != 0)
    {
        SetRunFlag(false);
        std::cout << "create thread error:" << strerror(nErr) << std::endl;
        goto Exit0;
    }
    
    bResult = true;
Exit0:
    
    return bResult;
}

bool KDBClient::Init(bool bMySQL, const char* pcszDBName, const std::vector<KConnectInfo>& vecConnectInfos)
{
    bool bResult = false;
    bool bRet = false;
    int nErr = 0;
    
    SetMySQLFlag(bMySQL);
    strcpy(m_szDBName, pcszDBName);
    m_vecConnectInfo.resize(vecConnectInfos.size());
    copy(vecConnectInfos.begin(), vecConnectInfos.end(), m_vecConnectInfo.begin());
    
    bRet = ConnectDB();
    if(!bRet)
    {
        goto Exit0;
    }
    
    SetRunFlag(true);
    nErr = pthread_create(&m_threadId, NULL, WorkThreadFunction, this);
    if(nErr != 0)
    {
        SetRunFlag(false);
        _ASSERT(nErr);
        goto Exit0;
    }
    
    bResult = true;
Exit0:
    return bResult;
}

bool KDBClient::UnInit()
{
    IKG_Buffer* pBuffer = NULL;
    SetRunFlag(false);
    pthread_join(m_threadId, NULL);
    
    _ASSERT(m_RequestQueue.size() == 0);
    while (!m_RequestQueue.empty()) {
        pBuffer = m_RequestQueue.front();
        SAFE_RELEASE(pBuffer);
        m_RequestQueue.pop_front();
    }
    
    _ASSERT(m_RespondQueue.size() == 0);
    while (!m_RespondQueue.empty()) {
        pBuffer = m_RespondQueue.front();
        SAFE_RELEASE(pBuffer);
        m_RespondQueue.pop_front();
    }
    
    return true;
}


int KDBClient::GetNetPacketReserved()
{
    //return g_pDBClientMgr->GetNetPacketReserved();
    return 0;
}

bool KDBClient::SetNetPacketReserved(char* pData)
{
    //return g_pDBClientMgr->SetNetPacketReserved(pData);
    return 0;
}

void* KDBClient::WorkThreadFunction(void *pvParam)
{
    KDBClient* pDBClient = (KDBClient*)(pvParam);
    _ASSERT(pDBClient);
    pDBClient->PreThreadActive();
    pDBClient->ThreadFunction();
    return ((void*)0);
}

void KDBClient::ThreadFunction()
{
    while (GetRunFlag())
    {
        DBThreadActivate();
        m_uPingTime += m_uSleepTime;
        KSLEEP(m_uSleepTime);
    }
    
    if(!m_bDatabaseLost)
    {
        DBThreadActivate();
    }
}

void KDBClient::DBThreadActivate()
{
    _ASSERT(!m_bDatabaseLost);
    IKG_Buffer*  pPackage = NULL;
    while (IsOpen())
    {
        SAFE_RELEASE(pPackage);
        pPackage = PopRequest();
        if(!pPackage)
            break;
        
        std::cout<< "dealing request..." << std::endl;
        OnRequest(pPackage);
    }
    
    SAFE_RELEASE(pPackage);
    
    if(m_uPingTime > KD_PING_INTERVAL)
    {
        Ping();
        m_uPingTime = 0;
    }
}

bool KDBClient::PushRequest(IKG_Buffer *pBuffer)
{
    bool bResult = false;
    if(!pBuffer)
    {
        goto Exit0;
    }
    
    pthread_mutex_lock(&m_mutex);
    
    pBuffer->AddRef();
    m_RequestQueue.push_back(pBuffer);
    pthread_mutex_unlock(&m_mutex);
    
    std::cout << "push request......." << m_RequestQueue.size() << std::endl;
Exit0:
    bResult = true;
    
    return bResult;
}

IKG_Buffer* KDBClient::PopRequest()
{
    IKG_Buffer* piBuffer = NULL;
    
    pthread_mutex_lock(&m_mutex);
    
    if (!m_RequestQueue.empty())
    {
        std::cout << "request size:" << m_RequestQueue.size() << std::endl;
        piBuffer = m_RequestQueue.front();
        m_RequestQueue.pop_front();
    }
    
    pthread_mutex_unlock(&m_mutex);
    
    return piBuffer;
}

bool KDBClient::PushRespond(IKG_Buffer *pBuffer)
{
    bool bResult = false;
    if(!pBuffer)
    {
        goto Exit0;
    }
    
    pthread_mutex_lock(&m_mutex);
    
    pBuffer->AddRef();
    m_RespondQueue.push_back(pBuffer);
    pthread_mutex_unlock(&m_mutex);
    
Exit0:
    bResult = true;
    
    return bResult;
}
    
IKG_Buffer* KDBClient::PopRespond()
{
    IKG_Buffer* piBuffer = NULL;
    
    pthread_mutex_lock(&m_mutex);
    
    if (!m_RespondQueue.empty())
    {
        std::cout << "response size:" << m_RespondQueue.size() << std::endl;
        piBuffer = m_RespondQueue.front();
        m_RespondQueue.pop_front();
    }
    
    pthread_mutex_unlock(&m_mutex);
    
    return piBuffer;
}

void KDBClient::Activate()
{
    IKG_Buffer* pPackage = NULL;
    while (true)
    {
        SAFE_RELEASE(pPackage);
        pPackage = PopRespond();
        if (!pPackage)
        {
            //std::cout << "response queue is null..." << std::endl;
            break;
        }
        OnResponsed(pPackage);
    }
    SAFE_RELEASE(pPackage);
}