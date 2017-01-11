//
//  game_logic_server.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include <exception>

#include "document.h"
#include "file_util.h"
#include "db_client_manager.hpp"
#include "game_logic_server.hpp"

using namespace rapidjson;

static void OnUVTimer(uv_timer_t *handle) {
    g_pDBClientMgr->Activate();
}

GameLogicServer::GameLogicServer()
{
    lua_engine.InitState(LuaEngine::SERVER_TYPE::LOGIC);
}

GameLogicServer::~GameLogicServer()
{
    
    cout << "Server Terminated" << endl;
}

GameLogicServer* GameLogicServer::GetInstance()
{
    static GameLogicServer server;
    return &server;
}

int GameLogicServer::Init(uv_loop_t* loop)
{
    string sz_config;
    bool is_ok = g_pFileUtil->ReadFile("config.json", sz_config);
    if(!is_ok)
    {
        cout << "read config failed." << endl;
        return 0;
    }
    
    Document json_doc;
    json_doc.Parse(sz_config.c_str());
    
    const Value& server_config = json_doc["listen"];
    string sz_ip = server_config["ip"].GetString();
    int port = server_config["port"].GetInt();
    const char* ip = sz_ip.c_str();
    
    SetIp(ip);
    SetPort(port);
    SetLoop(loop);
    
    uv_tcp_t& server = GetTcpServerHandler();
    uv_tcp_init(loop, &server);
    
    sockaddr_in& addr = GetSockAddrIn();
    uv_ip4_addr(ip, port, &addr);
    
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    
    int r = uv_listen((uv_stream_t*) &server, GetDefaultBackLog(),
                      [](uv_stream_t* server, int status)
                      {
                          GameLogicServer::GetInstance()->OnNewConnection(server, status);
                      });
    
    if (r) {
        fprintf(stderr, "Listen error %s: %s:%d\n", uv_strerror(r), ip, port);
        exit(1);
    }
    
    cout << "logic server listen: " << ip << ":" << port << endl;
    
    //数据管理定时器
    uv_timer_init(loop, &m_db_timer_req);
    uv_timer_start(&m_db_timer_req, OnUVTimer, 0, 100);
    
    return 1;
}

void GameLogicServer::test_throughput(uint64_t repeat)
{
    cout << "cost: " << repeat << ", total request:" << totol_request
            <<", avg:" << totol_request/repeat << endl;
}

void GameLogicServer::OnMsgRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf)
{
    
    if (nread == UV_EOF)
    {
        cout << "Client Disconnected" << endl;
        //to do: remove client handler
        //uv_close((uv_handle_t*)client, NULL);
    }
    else if (nread > 0)
    {
        std::string str = buf->base;
        _ProcessNetData(buf->base, nread);
    }
    
    Write(client, buf->base);
    free(buf->base);

}

void GameLogicServer::OnNewConnection(uv_stream_t *server, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    
    TCPSession new_session;
    new_session.connection = std::make_shared<uv_tcp_t>();
    
    uv_tcp_init(GetLoop(), new_session.connection.get());
    if (uv_accept(server, (uv_stream_t*)new_session.connection.get()) == 0) {
        
        uv_read_start((uv_stream_t*)new_session.connection.get(),
                      [](uv_handle_t* stream, size_t nread, uv_buf_t *buf)
                      {
                          GameLogicServer::GetInstance()->AllocBuffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          GameLogicServer::GetInstance()->OnMsgRecv(stream, nread, buf);
                      });
        
        uv_stream_t* key = (uv_stream_t*)new_session.connection.get();
        
        session_map_t& open_sessions = GetSessionMap();
        open_sessions.insert({key, new_session});
        
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}

void GameLogicServer::Write(uv_stream_t* client, string message){
    int len = (int)message.length();
    
    char buffer[100];
    uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
    
    buf.len = len;
    buf.base = (char*)message.c_str();

    uv_write(&m_write_req, client, &buf, 1,
             [](uv_write_t *req, int status)
             {
                 GameLogicServer::GetInstance()->OnWrite(req, status);
             });
}

void GameLogicServer::OnWrite(uv_write_t *req, int status){
    if(status == 0)
    {
        //cout << "OnWrite" << endl;
    }
}

void GameLogicServer::OnDBResponse(KRESOOND_COMMON* pCommonResponse)
{
    //调用lua处理回调
    int nDataLen = pCommonResponse->nDataLen;
    char* szData = new char[nDataLen];
    memcpy(szData, pCommonResponse->data, (size_t)nDataLen);
    lua_engine.RedisCallLua(pCommonResponse->uUserId, pCommonResponse->uEventType, szData);
}

bool GameLogicServer::_ProcessNetData(const char* pData, size_t uRead)
{
    bool bResult = false;
    unsigned int uWrite = 0;
    if(!(pData && uRead > 0))
        goto Exit0;
    do
    {
        _ASSERT(m_pRecvPacket);
        if(!(m_pRecvPacket->Write(pData, (unsigned int)uRead, &uWrite)))
            goto Exit0;
        if (m_pRecvPacket->IsValid())
        {
            IKG_Buffer* pBuffer = NULL;
            bool bRet = m_pRecvPacket->GetData(&pBuffer);
            if(!(bRet && pBuffer))
                goto Exit0;
            
            Message msg;
            if(msg.ParseFromArray(pBuffer->GetData(), pBuffer->GetSize()))
            {
                unsigned int uEventType = msg.event_type();
                string szParam = msg.data();
                
                lua_engine.CallLua(uEventType, szParam);
            }
            
            m_pRecvPacket->Reset();
        }
        pData += uWrite;
        uRead -= uWrite;
        if (uRead > 0)
            continue;
        break;
    } while (true);
    bResult = true;
Exit0:
    return bResult;
}