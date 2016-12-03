//
//  game_logic_server.hpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#pragma once

#ifndef game_logic_server_hpp
#define game_logic_server_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <time.h>

#include <map>
#include <uv.h>

#include "google.pb.h"

#include "lua_engine.hpp"
#include "tcp_client.hpp"
#include "tcp_base_server.hpp"

using namespace std;
using namespace google;

class GameLogicServer : public TCPBaseServer {
    
public:
    
    GameLogicServer();
    
    ~GameLogicServer();
    
    GameLogicServer(const GameLogicServer& GameLogicServer);
    
    static GameLogicServer* GetInstance();
    
    //初始化
    int Init(uv_loop_t* loop, const char* ip, int port);
    
    //监听新连接
    void OnNewConnection(uv_stream_t *server, int status);
    
    //接收数据
    void OnMsgRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf);
    
    //回写数据
    void write(uv_stream_t* client, string msg);
    
    //回写数据的回调
    void OnWrite(uv_write_t* req, int status);
    
    //测试吞吐量
    void test_throughput(uint64_t repeat);
    
    uv_write_t m_write_req;
    
private:
    
    session_map_t open_sessions;
    
    LuaEngine lua_engine;
    
    int totol_request;
    
};

#endif /* game_logic_server_hpp */