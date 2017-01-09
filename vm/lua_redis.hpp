//
//  lua_redis.hpp
//  server
//
//  Created by 冯文斌 on 17/1/6.
//  Copyright © 2017年 kelvin. All rights reserved.
//

#ifndef lua_redis_hpp
#define lua_redis_hpp

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

int LuaRegisterRedis(lua_State* lua_state);
#endif /* lua_redis_hpp */
