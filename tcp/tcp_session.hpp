//
//  tcp_session.hpp
//  server
//
//  Created by 冯文斌 on 16/10/8.
//  Copyright © 2016年 冯文斌. All rights reserved.
//

#ifndef tcp_session_hpp
#define tcp_session_hpp

#include <stdio.h>
#pragma once

#include <uv.h>

#include <map>
#include <queue>
#include <memory>

#endif /* tcp_session_hpp */

class TCPSession
{
public:
    
    TCPSession();
    ~TCPSession();
    
    std::shared_ptr<uv_tcp_t> connection;
    std::shared_ptr<uv_timer_t> activity_timer;
    
};
