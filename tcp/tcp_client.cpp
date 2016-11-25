//
//  tcp_client.cpp
//  client
//
//  Created by 冯文斌 on 16/8/18.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "tcp_client.hpp"

TCPClient::TCPClient()
{
    
}

TCPClient* TCPClient::get_instance()
{
    static TCPClient client;
    return &client;
}

int TCPClient::init(uv_loop_t* loop, const char* ip, int port)
{
    m_ip = ip;
    m_port = port;
    
    m_loop = loop;
    
    uv_tcp_init(m_loop, &m_client);
    uv_ip4_addr(m_ip, m_port, &m_dest);
    uv_tcp_connect(&m_connect_req, &m_client, (const struct sockaddr*)&m_dest,
                   [](uv_connect_t *req, int status)
                   {
                       TCPClient::get_instance()->on_connect(req, status);
                   });
    
    cout << "client connect to " << m_ip << ":" << m_port << endl;
    
    return uv_run(m_loop, UV_RUN_DEFAULT);
}

TCPClient::TCPClient(const TCPClient& TCPClient){
    
}

void TCPClient::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void TCPClient::on_write(uv_write_t *req, int status){
    
    if (status == -1) {
        fprintf(stderr, "error on_write");
        return;
    }
    
    if (status == 0) {
        cout << "send to server succeed!" << endl;
    }
}

void TCPClient::on_connect(uv_connect_t *req, int status) {
    
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
        return;
    }
 
}

void TCPClient::write(string message){
    int len = (int)message.length();
    cout << "write: " << message.c_str() << endl;
    
    char buffer[100];
    uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
    
    buf.len = len;
    buf.base = (char*)message.c_str();
    
    int buf_count = 1;
    
    int ret = uv_write(&m_write_req, m_connect_req.handle, &buf, buf_count,
                       [](uv_write_t *req, int status)
                       {
                           TCPClient::get_instance()->on_write(req, status);
                       });
    
    if(ret == 0) {
        //send to server succeed
    }
}

TCPClient::~TCPClient(){
    uv_loop_close(m_loop);
}