//
//  tcp_server_base.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#include "tcp_base_server.hpp"

static const int DEFAULT_BACKLOG = 1000;

TCPBaseServer::TCPBaseServer()
{
    
}

TCPBaseServer::~TCPBaseServer()
{
    uv_loop_close(m_loop);
    cout << "Server Terminated" << endl;
}

TCPBaseServer* TCPBaseServer::get_instance()
{
    static TCPBaseServer server;
    return &server;
}

void TCPBaseServer::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

int TCPBaseServer::init(uv_loop_t* loop, const char* ip, int port)
{
    m_ip = ip;
    m_port = port;
    m_loop = loop;
    
    uv_tcp_init(loop, &m_server);
    
    uv_ip4_addr(ip, m_port, &m_addr);
    
    uv_tcp_bind(&m_server, (const struct sockaddr*)&m_addr, 0);
    
    int r = uv_listen((uv_stream_t*) &m_server, DEFAULT_BACKLOG,
                      [](uv_stream_t* server, int status)
                      {
                          TCPBaseServer::get_instance()->on_new_connection(server, status);
                      });
    
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 0;
    }
    
    cout << "server listen: " << m_ip << ":" << m_port << endl;
    return 1;
}

void TCPBaseServer::on_msg_recv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
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

void TCPBaseServer::on_new_connection(uv_stream_t *server, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    
    TCPSession new_session;
    new_session.connection = std::make_shared<uv_tcp_t>();
    
    uv_tcp_init(m_loop, new_session.connection.get());
    if (uv_accept(server, (uv_stream_t*)new_session.connection.get()) == 0) {
        
        uv_read_start((uv_stream_t*)new_session.connection.get(),
                      [](uv_handle_t* stream, size_t nread, uv_buf_t *buf)
                      {
                          TCPBaseServer::get_instance()->alloc_buffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          TCPBaseServer::get_instance()->on_msg_recv(stream, nread, buf);
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
