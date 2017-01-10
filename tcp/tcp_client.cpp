//
//  tcp_client.cpp
//  client
//
//  Created by 冯文斌 on 16/8/18.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#include "kmacros.h"
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
    
    return ret;
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

void TCPClient::OnConnect(uv_connect_t *pServer, int nStatus) {
    
    if (nStatus < 0) {
        cout << "TCP Client connect error: " << uv_strerror(nStatus) << endl;
        exit(1);
    }
    
    if (nStatus == -1) {
        fprintf(stderr, "error OnConnect");
        return;
    }
    
    int iret = uv_read_start(pServer->handle, AllocBuffer,
                             [](uv_stream_t* req, ssize_t nread, const uv_buf_t *buf)
                             {
                                 TCPClient::GetInstance()->OnMsgRecv(req, nread, buf);
                             });
    
    if(!iret)
    {
        //尝试重连
    }
 
}

void TCPClient::OnMsgRecv(uv_stream_t* pServer, ssize_t nread, const uv_buf_t *buf)
{
    if (nread == UV_EOF)
    {
        cout << "Server Disconnected" << endl;
    }
    else if (nread > 0)
    {
        std::string str = buf->base;
    }
    
    free(buf->base);
}

void TCPClient::Write(const string& message){
    unsigned int uMsgSize = (unsigned int)message.length();
    unsigned int uHeadSize = sizeof(unsigned int);
    
    char* pvBuffer = NULL;
    unsigned int uPacketLen = uHeadSize + uMsgSize;
    pvBuffer = (char*)malloc(uPacketLen);
    memset(pvBuffer, 0, uPacketLen);
    
    memcpy(pvBuffer, &uPacketLen, uMsgSize);
    memcpy(pvBuffer + uMsgSize, message.c_str(), uMsgSize);
    
    uv_write_t* pWriteReq = NULL;
    pWriteReq = new uv_write_t;
    pWriteReq->data = pvBuffer;
    
    uv_buf_t buf = uv_buf_init(pvBuffer, uPacketLen);
    int ret = uv_write(pWriteReq, (uv_stream_t*)&m_client, &buf, 1,
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
