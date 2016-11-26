//
//  tcp_server_base.hpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#ifndef tcp_server_base_hpp
#define tcp_server_base_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <unistd.h>
#include <iostream>
#include <map>

#include "tcp_session.hpp"

using namespace std;

typedef std::map<uv_stream_t*, TCPSession> session_map_t;

class TCPBaseServer{
    
public:
    
    TCPBaseServer();
    
    virtual ~TCPBaseServer();
    
    TCPBaseServer(const TCPBaseServer& TCPBaseServer);
    
    static TCPBaseServer* get_instance();
    
    virtual int GetDefaultBackLog();
    
    virtual int init(uv_loop_t* loop, const char* ip, int port);
    
    virtual void on_new_connection(uv_stream_t *server, int status);
    
    virtual void on_msg_recv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    
    virtual void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    
    virtual void SetPort(int port);
    
    virtual int GetPort();
    
    virtual void SetIp(const char* ip);
    
    virtual const char* GetIp();
    
    virtual void SetLoop(uv_loop_t* loop);
    
    virtual uv_loop_t* GetLoop();
    
protected:
    
    uv_loop_t *m_loop;
    
    uv_tcp_t m_server;
    
    struct sockaddr_in m_addr;
    
    session_map_t open_sessions;
    
private:
    
    int m_port;
    
    const char *m_ip;
    
    int default_backlog = 1000;
};

#endif /* tcp_base_server_hpp */