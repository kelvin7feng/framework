//
//  tcp_client.hpp
//  client
//
//  Created by 冯文斌 on 16/9/5.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef tcp_client_hpp
#define tcp_client_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <unistd.h>
#include <iostream>

using namespace std;

class TCPClient{
    
public:
    
    TCPClient();
   
    virtual ~TCPClient();
    
    TCPClient(const TCPClient& TCPClient);
    
    static TCPClient* GetInstance();
    
    virtual int Init(uv_loop_t* loop, const char* ip, int port);
    
    virtual void SetPort(int port);
    
    virtual int GetPort();
    
    virtual void SetIp(const char* ip);
    
    virtual const char* GetIp();
    
    virtual void SetLoop(uv_loop_t* loop);
    
    virtual uv_loop_t* GetLoop();
    
    virtual void Write(const string& msg);
    
    virtual void OnMsgRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf);
    
    virtual void OnWrite(uv_write_t *req, int status);
    
    virtual void OnConnect(uv_connect_t *req, int status);
    
    static void AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    
protected:
    
    int m_port;
    
    const char* m_ip;
    
    uv_loop_t* m_loop;
    
    uv_tcp_t m_client;
    
    struct sockaddr_in m_dest;
    
    uv_connect_t m_connect_req;
    
    uv_write_t m_write_req;
};

#endif /* tcp_client_hpp */