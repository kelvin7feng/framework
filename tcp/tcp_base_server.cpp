//
//  tcp_server_base.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#include "tcp_base_server.hpp"

TCPBaseServer::TCPBaseServer()
{
    
}

TCPBaseServer::~TCPBaseServer()
{
    uv_loop_close(GetLoop());
    cout << "TCP Server Terminated" << endl;
}

TCPBaseServer* TCPBaseServer::GetInstance()
{
    static TCPBaseServer server;
    return &server;
}

void TCPBaseServer::AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

int TCPBaseServer::Init(uv_loop_t* loop, const char* ip, int port)
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
                          TCPBaseServer::GetInstance()->OnNewConnection(server, status);
                      });
    
    if (r) {
        fprintf(stderr, "Listen error %s: %s:%d\n", uv_strerror(r), ip, port);
        exit(1);
    }
    
    cout << "server listen: " << ip << ":" << port << endl;
    return 1;
}

void TCPBaseServer::OnMsgRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
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
            cout << "recv: " << str << endl;
        }
        
        free(buf->base);
    }
}

void TCPBaseServer::OnNewConnection(uv_stream_t *server, int status)
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
                          TCPBaseServer::GetInstance()->AllocBuffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          TCPBaseServer::GetInstance()->OnMsgRecv(stream, nread, buf);
                      });
        
        uv_stream_t* key = (uv_stream_t*)new_session.connection.get();
        open_sessions.insert({key, new_session});
        
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}

int TCPBaseServer::GetDefaultBackLog()
{
    return default_backlog;
}

void TCPBaseServer::SetPort(int port)
{
    m_port = port;
}

int TCPBaseServer::GetPort()
{
    return m_port;
}

void TCPBaseServer::SetIp(const char* ip)
{
    m_ip = ip;
}

const char* TCPBaseServer::GetIp()
{
    return m_ip;
}

void TCPBaseServer::SetLoop(uv_loop_t* loop)
{
    m_loop = loop;
}

uv_loop_t* TCPBaseServer::GetLoop()
{
    return m_loop;
}

void TCPBaseServer::SetTcpServerHandler(uv_tcp_t server)
{
    m_server = server;
}

uv_tcp_t TCPBaseServer::GetTcpServerHandler()
{
    return m_server;
}

void TCPBaseServer::SetSockAddrIn(sockaddr_in sockaddr_in_struct)
{
    m_addr = sockaddr_in_struct;
}

sockaddr_in TCPBaseServer::GetSockAddrIn()
{
    return m_addr;
}
