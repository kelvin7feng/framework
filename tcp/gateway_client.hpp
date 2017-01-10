//
//  gateway_client.hpp
//  server
//
//  Created by 冯文斌 on 17/1/10.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#ifndef gateway_client_hpp
#define gateway_client_hpp

#include <uv.h>
#include "tcp_client.hpp"

class GatewayClient : public TCPClient
{
public:
    GatewayClient();
    
    ~GatewayClient();
    
    GatewayClient(const GatewayClient& GatewayClient);
    
    static GatewayClient* GetInstance();
    
    //初始函数
    int Init(uv_loop_t* loop, const char* ip, int port);
    
    //连接后的回调函数
    void OnConnect(uv_connect_t *req, int status);
    
    //数据收到处理,转发给客户端
    void OnMsgRecv(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf);
    
    //转发到服务端函数
    void TransferToLogicServer(const char* pBuffer, ssize_t nRead);
    
    //转发到服务端的回调
    void OnTransferToLogicServer(uv_write_t *req, int status);
};

#endif /* gateway_client_hpp */
