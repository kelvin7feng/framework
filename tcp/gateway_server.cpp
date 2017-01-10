//
//  gateway_server.cpp
//  server
//
//  Created by 冯文斌 on 16/9/5.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#include "kmacros.h"
#include "gateway_client.hpp"
#include "gateway_server.hpp"

GatewayServer::GatewayServer()
{
    
}

GatewayServer::~GatewayServer()
{
    cout << "Gateway Server Terminated" << endl;
}

GatewayServer* GatewayServer::GetInstance()
{
    static GatewayServer server;
    return &server;
}

int GatewayServer::Init(uv_loop_t* loop, const char* ip, int port)
{
    SetIp(ip);
    SetPort(port);
    SetLoop(loop);
    
    uv_tcp_t& server = GetTcpServerHandler();
    uv_tcp_init(loop, &server);
    
    sockaddr_in& addr = GetSockAddrIn();
    uv_ip4_addr(ip, port, &addr);
    
    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    
    int ret = uv_listen((uv_stream_t*) &server, GetDefaultBackLog(),
                      [](uv_stream_t* server, int status)
                      {
                          GatewayServer::GetInstance()->OnNewConnection(server, status);
                      });

    
    if (ret) {
        fprintf(stderr, "Listen error %s: %s:%d\n", uv_strerror(ret), ip, port);
        exit(1);
    }
    
    cout << "gateway server listen " << ip << ":" << port << " succeed"<< endl;
    return ret;
}

void GatewayServer::OnMsgRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    
    session_map_t& open_sessions = GetSessionMap();
    auto connection_pos = open_sessions.find(client);
    if (connection_pos != open_sessions.end())
    {
        if (nread == UV_EOF)
        {
            cout << "Client Disconnected" << endl;
            RemoveClient(client);
        }
        else if (nread > 0)
        {
            if(m_pRecvPacket->CheckNetPacket(buf->base, (int)nread))
            {
                unsigned int nHandlerId = GetHandlerIdByHandler(client);
                AddHanderIdToPacket(nHandlerId, buf->base, (int)nread);
                GatewayClient* pGamewayClientToLogic = GatewayClient::GetInstance();
                pGamewayClientToLogic->TransferToLogicServer(buf->base, (int)nread);
            }
        }
        
        free(buf->base);
    }
    else
    {
        uv_read_stop(client);
        cout << "Unrecognized client. Disconnecting." << endl;;
    }
}

void GatewayServer::RemoveClient(uv_stream_t* client)
{
    session_map_t& open_sessions = GetSessionMap();
    auto connection_pos = open_sessions.find(client);
    if (connection_pos != open_sessions.end())
    {
        uv_close((uv_handle_t*)connection_pos->second.connection.get(),
                 [] (uv_handle_t* handle)
                 {
                     GatewayServer::GetInstance()->OnConnectionClose(handle);
                 });
    }
}

void GatewayServer::OnConnectionClose(uv_handle_t* handle)
{
    session_map_t& open_sessions = GatewayServer::GetInstance()->GetSessionMap();
    open_sessions.erase((uv_stream_t*)handle);
}

void GatewayServer::OnNewConnection(uv_stream_t *server, int status)
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
                          GatewayServer::GetInstance()->AllocBuffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          GatewayServer::GetInstance()->OnMsgRecv(stream, nread, buf);
                      });
        
        AddSession(new_session);
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}

void GatewayServer::AddHanderIdToPacket(unsigned int nHandlerId, void* pBuffer, unsigned int uSize)
{
    unsigned int uHandlerIdStart = sizeof(unsigned int) * 2;//第一个是长度，第二个是事件，第三个是句柄Id,size为unsigned int
    if(uSize < uHandlerIdStart + sizeof(unsigned int))
    {
        cout << "packer is error!!!!";
    }
    cout << "handler id:" << nHandlerId << endl;
    memcpy((char*)pBuffer + uHandlerIdStart, &nHandlerId, sizeof(unsigned int));
}

bool GatewayServer::_ProcessNetData(const char* pData, size_t uSize)
{
    return false;
}