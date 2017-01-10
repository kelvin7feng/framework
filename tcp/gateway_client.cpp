//
//  gateway_client.cpp
//  server
//
//  Created by 冯文斌 on 17/1/10.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#include "kmacros.h"
#include "gateway_client.hpp"
#include "gateway_server.hpp"

GatewayClient::GatewayClient()
{
    
}

GatewayClient::~GatewayClient()
{
    
}

GatewayClient* GatewayClient::GetInstance()
{
    static GatewayClient client;
    return &client;
}

GatewayClient::GatewayClient(const GatewayClient& GatewayClient){
    
}

int GatewayClient::Init(uv_loop_t* loop, const char* ip, int port)
{
    SetIp(ip);
    SetPort(port);
    SetLoop(loop);
    
    uv_tcp_init(loop, &m_client);
    uv_ip4_addr(ip, port, &m_dest);
    int ret = uv_tcp_connect(&m_connect_req, &m_client, (const struct sockaddr*)&m_dest,
                             [](uv_connect_t *req, int status)
                             {
                                 GatewayClient::GetInstance()->OnConnect(req, status);
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

void GatewayClient::OnConnect(uv_connect_t *pServer, int nStatus) {
    
    if (nStatus < 0) {
        cout << "GatewayClient connect error: " << uv_strerror(nStatus) << endl;
        exit(1);
    }
    
    if (nStatus == -1) {
        fprintf(stderr, "error GatewayClient OnConnect");
        return;
    }
    
    int iret = uv_read_start(pServer->handle, AllocBuffer,
                             [](uv_stream_t* req, ssize_t nread, const uv_buf_t *buf)
                             {
                                 GatewayClient::GetInstance()->OnMsgRecv(req, nread, buf);
                             });
    
    if(!iret)
    {
        //尝试重连
    }
    
}

void GatewayClient::OnMsgRecv(uv_stream_t* pServer, ssize_t nread, const uv_buf_t *buf)
{
    if (nread == UV_EOF)
    {
        cout << "Server Disconnected" << endl;
    }
    else if (nread > 0)
    {
        std::string str = buf->base;
    }
    
    //通过Gateway回传给对应的客户端
    
    free(buf->base);
}

//转发函数
void GatewayClient::TransferToLogicServer(const char* pBuffer, ssize_t nRead){
    
    char* pvBuffer = NULL;
    pvBuffer = (char*)malloc(nRead);
    memset(pvBuffer, 0, nRead);
    memcpy(pvBuffer, pBuffer, nRead);
    
    uv_write_t* pWriteReq = NULL;
    pWriteReq = new uv_write_t;
    pWriteReq->data = pvBuffer;
    
    uv_buf_t buf = uv_buf_init(pvBuffer, (unsigned int)nRead);
    int ret = uv_write(pWriteReq, (uv_stream_t*)&m_client, &buf, 1,
                       [](uv_write_t *req, int status)
                       {
                           GatewayClient::GetInstance()->OnTransferToLogicServer(req, status);
                       });
    
    if(ret != 0) {
        SAFE_DELETE(pWriteReq);
    }
}

//转发到服务端的回调
void GatewayClient::OnTransferToLogicServer(uv_write_t *req, int status){
    
    if (status < 0) {
        cout << "TCP Client write error: " << uv_strerror(status) << endl;
        return;
    }
    
    if (status == 0) {
        cout << "transfer to server succeed!" << endl;
    }
}