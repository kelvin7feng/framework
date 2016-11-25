//
//  game_logic_server.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "game_logic_server.hpp"

GameLogicServer::GameLogicServer()
{
    lua_engine.InitState(CLuaEngine::SERVER_TYPE::LOGIC);
}

GameLogicServer::~GameLogicServer()
{
    uv_loop_close(m_loop);
    cout << "Server Terminated" << endl;
}

GameLogicServer* GameLogicServer::get_instance()
{
    static GameLogicServer server;
    return &server;
}

int GameLogicServer::init(uv_loop_t* loop, const char* ip, int port)
{
    m_ip = ip;
    m_port = port;
    m_loop = loop;
    
    uv_tcp_init(loop, &m_server);
    
    uv_ip4_addr(ip, m_port, &m_addr);
    
    uv_tcp_bind(&m_server, (const struct sockaddr*)&m_addr, 0);
    
    int r = uv_listen((uv_stream_t*) &m_server, GetDefaultBackLog(),
                      [](uv_stream_t* server, int status)
                      {
                          GameLogicServer::get_instance()->on_new_connection(server, status);
                      });
    
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 0;
    }
    
    cout << "logic server listen: " << m_ip << ":" << m_port << endl;
    return 1;
}

void GameLogicServer::test_throughput(uint64_t repeat)
{
    cout << "cost: " << repeat << ", total request:" << totol_request
            <<", avg:" << totol_request/repeat << endl;
}

void GameLogicServer::on_msg_recv(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf)
{
    
    if (nread == UV_EOF)
    {
        cout << "Client Disconnected" << endl;
    }
    else if (nread > 0)
    {
        std::string str = buf->base;
        Message msg;
        
        //to do:解析失败,则不处理
        msg.ParseFromString(str);
        
        //int server_id = msg.server_id();
        //int user_id = msg.player_id();
        string request = msg.pdata();
        
        lua_engine.CallLua(request);
    }
    
    write(client, buf->base);
    free(buf->base);

}

void GameLogicServer::on_new_connection(uv_stream_t *server, int status)
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
                          GameLogicServer::get_instance()->alloc_buffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          GameLogicServer::get_instance()->on_msg_recv(stream, nread, buf);
                      });
        
        uv_stream_t* key = (uv_stream_t*)new_session.connection.get();
        open_sessions.insert({key, new_session});
        
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}

void GameLogicServer::write(uv_stream_t* client, string message){
    int len = (int)message.length();
    
    char buffer[100];
    uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
    
    buf.len = len;
    buf.base = (char*)message.c_str();

    uv_write(&m_write_req, client, &buf, 1,
             [](uv_write_t *req, int status)
             {
                 GameLogicServer::get_instance()->on_write(req, status);
             });
}

void GameLogicServer::on_write(uv_write_t *req, int status){
    if(status == 0)
    {
        //cout << "on_write" << endl;
    }
}