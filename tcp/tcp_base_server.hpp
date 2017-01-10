//
//  tcp_server_base.hpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#ifndef tcp_server_base_hpp
#define tcp_server_base_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include <unistd.h>
#include <iostream>
#include <map>

#include "knetpacket.h"
#include "tcp_session.hpp"

using namespace std;

typedef std::map<uv_stream_t*, TCPSession> session_map_t;
typedef std::map<unsigned int, uv_stream_t*> id_map_to_handler_t;
typedef std::map<uv_stream_t*, unsigned int> handler_map_to_id_t;

class TCPBaseServer{
    
public:
    
    TCPBaseServer();
    
    virtual ~TCPBaseServer();
    
    TCPBaseServer(const TCPBaseServer& TCPBaseServer);
    
    static TCPBaseServer* GetInstance();
    
    virtual int GetDefaultBackLog();
    
    virtual int Init(uv_loop_t* loop, const char* ip, int port);
    
    virtual void OnNewConnection(uv_stream_t *server, int status);
    
    virtual void OnMsgRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    
    virtual void AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    
    virtual void SetPort(int port);
    
    virtual int GetPort();
    
    virtual void SetIp(const char* ip);
    
    virtual const char* GetIp();
    
    virtual void SetLoop(uv_loop_t* loop);
    
    virtual uv_loop_t* GetLoop();
    
    virtual void SetTcpServerHandler(uv_tcp_t server);
    
    virtual uv_tcp_t& GetTcpServerHandler();
    
    virtual void SetSockAddrIn(sockaddr_in sockaddr_in_struct);
    
    virtual sockaddr_in& GetSockAddrIn();
    
    virtual session_map_t& GetSessionMap();
    
    unsigned int GetHandlerIdByHandler(uv_stream_t* pKey);
    
    virtual id_map_to_handler_t& GetIdToHandlerMap();
    
    virtual handler_map_to_id_t& GetHandlerToIdMap();
    
    virtual void AddSession(TCPSession session);
    
    virtual void RemoveClient(uv_stream_t* client);
    
    virtual void OnConnectionClose(uv_handle_t* handle);
    
    virtual void SendData(const char* pBuffer, unsigned int uSize);
    
    virtual void OnSendData(uv_write_t *pReq, int nStatus);
    
protected:
    
    virtual bool _ProcessNetData(const char* pData, size_t uSize);
    
    IKNetPacket* m_pRecvPacket;
    
    uv_tcp_t m_server;
    
private:
    
    int m_nSessionId;
    
    int m_port;
    
    const char *m_ip;
    
    int m_default_backlog;
    
    struct sockaddr_in m_addr;
    
    session_map_t open_sessions;
    
    id_map_to_handler_t m_id_to_handler_map;
    
    handler_map_to_id_t m_hander_to_id_map;
    
    uv_loop_t *m_loop;
    
};

#endif /* tcp_base_server_hpp */