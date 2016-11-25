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

#endif /* tcp_client_hpp */

using namespace std;

class TCPClient{
    
public:
    
    TCPClient();
   
    ~TCPClient();
    
    TCPClient(const TCPClient& TCPClient);
    
    static TCPClient* get_instance();
    
    int init(uv_loop_t* loop, const char* ip, int port);
    
    void write(string msg);
    
protected:
    
    void on_write(uv_write_t *req, int status);
    
    void on_connect(uv_connect_t *req, int status);
    
    static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    
private:
    
    int m_port;
    
    const char* m_ip;
    
    uv_loop_t* m_loop;
    
    uv_tcp_t m_client;
    
    struct sockaddr_in m_dest;
    
    uv_connect_t m_connect_req;
    
    uv_write_t m_write_req;
};