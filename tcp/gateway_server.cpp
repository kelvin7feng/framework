//
//  gateway_server.cpp
//  server
//
//  Created by 冯文斌 on 16/9/5.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

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
    
    uv_tcp_init(loop, &m_server);
    
    uv_ip4_addr(ip, port, &m_addr);
    
    uv_tcp_bind(&m_server, (const struct sockaddr*)&m_addr, 0);
    
    int r = uv_listen((uv_stream_t*) &m_server, GetDefaultBackLog(),
                      [](uv_stream_t* server, int status)
                      {
                          GatewayServer::GetInstance()->OnNewConnection(server, status);
                      });

    
    if (r) {
        fprintf(stderr, "Listen error %s: %s:%d\n", uv_strerror(r), ip, port);
        exit(1);
    }
    
    cout << "gateway server listen " << ip << ":" << port << " succeed"<< endl;
    return 1;
}

void GatewayServer::OnMsgRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    
    auto connection_pos = open_sessions.find(client);
    if (connection_pos != open_sessions.end())
    {
        if (nread == UV_EOF)
        {
            cout << "Client Disconnected" << endl;
        }
        else if (nread > 0)
        {
            std::string str = buf->base;
            Message msg;
            msg.ParseFromString(str);
            
            TCPClient* game_logic_server = TCPClient::GetInstance();
            game_logic_server->write(str);
        }
        
        free(buf->base);
    }
    
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
        
        uv_stream_t* key = (uv_stream_t*)new_session.connection.get();
        open_sessions.insert({key, new_session});
        
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}
