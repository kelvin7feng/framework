//
//  gateway_server.hpp
//  server
//
//  Created by 冯文斌 on 16/9/5.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#ifndef gateway_server_hpp
#define gateway_server_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include <map>
#include <uv.h>

#include "tcp_client.hpp"
#include "tcp_base_server.hpp"

#include "google.pb.h"

using namespace std;
using namespace google;

class GatewayServer : public TCPBaseServer {
    
public:
    
    GatewayServer();
    
    ~GatewayServer();
    
    GatewayServer(const GatewayServer& GatewayServer);
    
    static GatewayServer* get_instance();
    
    //初始化
    int init(uv_loop_t* loop, const char* ip, int port);
    
    //监听新建连接
    void on_new_connection(uv_stream_t *server, int status);
    
    //监听接收数据
    void on_msg_recv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    
protected:
    
    session_map_t open_sessions;
    
};

#endif /* gateway_server_hpp */