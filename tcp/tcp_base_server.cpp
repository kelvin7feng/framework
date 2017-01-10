//
//  tcp_server_base.cpp
//  server
//
//  Created by 冯文斌 on 16/10/10.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#include "kmacros.h"
#include "tcp_base_server.hpp"

TCPBaseServer::TCPBaseServer()
{
    m_nSessionId = 0;
    
    m_port = 0;
    
    m_default_backlog = 1000;

    m_pRecvPacket = KG_CreateCommonPackage();
}

TCPBaseServer::~TCPBaseServer()
{
    uv_loop_close(GetLoop());
    cout << "TCP Server Terminated" << endl;
}

TCPBaseServer* TCPBaseServer::GetInstance()
{
    static TCPBaseServer server;
    return &server;
}

void TCPBaseServer::AllocBuffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

int TCPBaseServer::Init(uv_loop_t* loop, const char* ip, int port)
{
    SetIp(ip);
    SetPort(port);
    SetLoop(loop);
    
    uv_tcp_init(loop, &m_server);
    
    uv_ip4_addr(ip, port, &m_addr);
    
    uv_tcp_bind(&m_server, (const struct sockaddr*)&m_addr, 0);
    
    int r = uv_listen((uv_stream_t*) &m_server, GetDefaultBackLog(),
                      [](uv_stream_t* server, int status)
                      {
                          TCPBaseServer::GetInstance()->OnNewConnection(server, status);
                      });
    
    if (r) {
        fprintf(stderr, "Listen error %s: %s:%d\n", uv_strerror(r), ip, port);
        exit(1);
    }
    
    cout << "server listen: " << ip << ":" << port << endl;
    return 1;
}

void TCPBaseServer::OnMsgRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    auto connection_pos = open_sessions.find(client);
    if (connection_pos != open_sessions.end())
    {
        if (nread == UV_EOF)
        {
            cout << "Client Disconnected" << endl;
            RemoveClient(client);
        }
        else if (nread > 0)
        {
            std::string str = buf->base;
            cout << "recv: " << str << endl;
        }
        
        free(buf->base);
    }
}

void TCPBaseServer::OnNewConnection(uv_stream_t *server, int status)
{
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        return;
    }
    
    TCPSession new_session;
    new_session.connection = std::make_shared<uv_tcp_t>();
    
    uv_tcp_init(GetLoop(), new_session.connection.get());
    if (uv_accept(server, (uv_stream_t*)new_session.connection.get()) == 0) {
        
        uv_read_start((uv_stream_t*)new_session.connection.get(),
                      [](uv_handle_t* stream, size_t nread, uv_buf_t *buf)
                      {
                          TCPBaseServer::GetInstance()->AllocBuffer(stream, nread, buf);
                      },
                      [](uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
                      {
                          TCPBaseServer::GetInstance()->OnMsgRecv(stream, nread, buf);
                      });
        
        uv_stream_t* key = (uv_stream_t*)new_session.connection.get();
        open_sessions.insert({key, new_session});
        
    }
    else {
        uv_close((uv_handle_t*)new_session.connection.get(), NULL);
    }
}

void TCPBaseServer::RemoveClient(uv_stream_t* client)
{
    auto connection_pos = open_sessions.find(client);
    if (connection_pos != open_sessions.end())
    {
        uv_close((uv_handle_t*)connection_pos->second.connection.get(),
                 [] (uv_handle_t* handle)
                 {
                     TCPBaseServer::GetInstance()->OnConnectionClose(handle);
                 });
    }
}

void TCPBaseServer::OnConnectionClose(uv_handle_t* handle)
{
    cout << "release connection of client..." << endl;
    open_sessions.erase((uv_stream_t*)handle);
}

void TCPBaseServer::SendData(const char* pBuffer, unsigned int uSize){
    
    char* pvBuffer = NULL;
    unsigned int uPacketLen = KD_PACKAGE_LEN_SIZE + uSize;
    pvBuffer = (char*)malloc(uPacketLen);
    memset(pvBuffer, 0, uPacketLen);
    
    memcpy(pvBuffer, &uPacketLen, KD_PACKAGE_LEN_SIZE);
    memcpy(pvBuffer + KD_PACKAGE_LEN_SIZE, pBuffer, uSize);
    
    uv_write_t* pWriteReq = NULL;
    pWriteReq = new uv_write_t;
    pWriteReq->data = pvBuffer;
    
    uv_buf_t pUvBuf = uv_buf_init(pvBuffer, uPacketLen);
    uv_write(pWriteReq, (uv_stream_t*)&m_server, &pUvBuf, 1,
             [](uv_write_t *pReq, int nStatus)
             {
                 TCPBaseServer::GetInstance()->OnSendData(pReq, nStatus);
             });
}

void TCPBaseServer::OnSendData(uv_write_t *pReq, int nStatus){
    
    if (nStatus == -1) {
        fprintf(stderr, "error on_write");
        return;
    }
    
    if (nStatus == 0) {
        cout << "send to server succeed!" << endl;
    }
}

int TCPBaseServer::GetDefaultBackLog()
{
    return m_default_backlog;
}

void TCPBaseServer::SetPort(int port)
{
    m_port = port;
}

int TCPBaseServer::GetPort()
{
    return m_port;
}

void TCPBaseServer::SetIp(const char* ip)
{
    m_ip = ip;
}

const char* TCPBaseServer::GetIp()
{
    return m_ip;
}

void TCPBaseServer::SetLoop(uv_loop_t* loop)
{
    m_loop = loop;
}

uv_loop_t* TCPBaseServer::GetLoop()
{
    return m_loop;
}

void TCPBaseServer::SetTcpServerHandler(uv_tcp_t server)
{
    m_server = server;
}

uv_tcp_t& TCPBaseServer::GetTcpServerHandler()
{
    return m_server;
}

void TCPBaseServer::SetSockAddrIn(sockaddr_in sockaddr_in_struct)
{
    m_addr = sockaddr_in_struct;
}

sockaddr_in& TCPBaseServer::GetSockAddrIn()
{
    return m_addr;
}

session_map_t& TCPBaseServer::GetSessionMap()
{
    return open_sessions;
}

unsigned int TCPBaseServer::GetHandlerIdByHandler(uv_stream_t* pKey)
{
    unsigned int uHandlerId = 0;
    handler_map_to_id_t& mapHandlerToId = GetHandlerToIdMap();
    auto handlerToId = mapHandlerToId.find(pKey);
    if(handlerToId != mapHandlerToId.end())
    {
        uHandlerId = handlerToId->second;
    }
    
    return uHandlerId;
}

id_map_to_handler_t& TCPBaseServer::GetIdToHandlerMap()
{
    return m_id_to_handler_map;
}

handler_map_to_id_t& TCPBaseServer::GetHandlerToIdMap()
{
    return m_hander_to_id_map;
}

void TCPBaseServer::AddSession(TCPSession session)
{
    uv_stream_t* key = (uv_stream_t*)session.connection.get();
    session_map_t& open_sessions = GetSessionMap();
    open_sessions.insert({key, session});
    m_nSessionId ++;
    
    id_map_to_handler_t& mapIdToHandler = GetIdToHandlerMap();
    mapIdToHandler.insert({m_nSessionId, key});
    
    handler_map_to_id_t& mapHandlerToId = GetHandlerToIdMap();
    mapHandlerToId.insert({key, m_nSessionId});
}


bool TCPBaseServer::_ProcessNetData(const char* pData, size_t uSize)
{
    return false;
}
