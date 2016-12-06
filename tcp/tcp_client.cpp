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
    m_port = 0;
}

TCPClient* TCPClient::GetInstance()
{
    static TCPClient client;
    return &client;
}

int TCPClient::Init(uv_loop_t* loop, const char* ip, int port)
{
    SetIp(ip);
    SetPort(port);
    SetLoop(loop);
    
    uv_tcp_init(loop, &m_client);
    uv_ip4_addr(ip, port, &m_dest);
    int ret = uv_tcp_connect(&m_connect_req, &m_client, (const struct sockaddr*)&m_dest,
                   [](uv_connect_t *req, int status)
                   {
                       TCPClient::GetInstance()->OnConnect(req, status);
                   });
    
    if(ret < 0)
    {
        cout << "client connect error: " << ip << ":" << port << endl;
        exit(1);
    }
    
    if(ret == 0)
    {
        cout << "client connect to " << ip << ":" << port << " succeed"<< endl;
    }
    
    return uv_run(loop, UV_RUN_DEFAULT);
}

TCPClient::TCPClient(const TCPClient& TCPClient){
    
}

void TCPClient::AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void TCPClient::OnWrite(uv_write_t *req, int status){
    
    if (status < 0) {
        cout << "TCP Client write error: " << uv_strerror(status) << endl;
        return;
    }
    
    if (status == 0) {
        cout << "send to server succeed!" << endl;
    }
}

void TCPClient::OnConnect(uv_connect_t *req, int status) {
    
    if (status < 0) {
        cout << "TCP Client connect error: " << uv_strerror(status) << endl;
        exit(1);
    }
 
}

void TCPClient::Write(string message){
    int len = (int)message.length();
    cout << "write: " << message.c_str() << endl;
    
    char buffer[100];
    uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));
    
    buf.len = len;
    buf.base = (char*)message.c_str();
    
    int buf_count = 1;
    
    if(m_write_req.error < 0)
    {
        cout << "m_write_req error:" << m_write_req.error << endl;
        cout << "try to reconnect..." << endl;
    }
    
    int ret = uv_write(&m_write_req, m_connect_req.handle, &buf, buf_count,
                       [](uv_write_t *req, int status)
                       {
                           TCPClient::GetInstance()->OnWrite(req, status);
                       });
    
    if(ret == 0) {
        //send to server succeed
    }
}

TCPClient::~TCPClient(){
    uv_loop_close(m_loop);
}

void TCPClient::SetPort(int port)
{
    m_port = port;
}

int TCPClient::GetPort()
{
    return m_port;
}

void TCPClient::SetIp(const char* ip)
{
    m_ip = ip;
}

const char* TCPClient::GetIp()
{
    return m_ip;
}

void TCPClient::SetLoop(uv_loop_t* loop)
{
    m_loop = loop;
}

uv_loop_t* TCPClient::GetLoop()
{
    return m_loop;
}
