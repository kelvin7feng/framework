/*
** Lua binding: player
** Generated automatically by tolua++-1.0.92 on Mon Nov 28 10:47:14 2016.
*/

#ifndef __cplusplus
#include "stdlib.h"
#endif
#include "string.h"

#include "tolua++.h"

/* Exported function */
TOLUA_API int  tolua_player_open (lua_State* tolua_S);


/* function to register type */
static void tolua_reg_types (lua_State* tolua_S)
{
}

/* function: Player::Player */
#ifndef TOLUA_DISABLE_tolua_player_Player__Player00
static int tolua_player_Player__Player00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Player::Player();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Player__Player'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Player::~Player */
#ifndef TOLUA_DISABLE_tolua_player_Player__~Player00
static int tolua_player_Player__~Player00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Player::~Player();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Player__~Player'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* function: Player::Run */
#ifndef TOLUA_DISABLE_tolua_player_Player__Run00
static int tolua_player_Player__Run00(lua_State* tolua_S)
{
#ifndef TOLUA_RELEASE
 tolua_Error tolua_err;
 if (
     !tolua_isnoobj(tolua_S,1,&tolua_err)
 )
  goto tolua_lerror;
 else
#endif
 {
  {
   Player::Run();
  }
 }
 return 0;
#ifndef TOLUA_RELEASE
 tolua_lerror:
 tolua_error(tolua_S,"#ferror in function 'Player__Run'.",&tolua_err);
 return 0;
#endif
}
#endif //#ifndef TOLUA_DISABLE

/* Open function */
TOLUA_API int tolua_player_open (lua_State* tolua_S)
{
 tolua_open(tolua_S);
 tolua_reg_types(tolua_S);
 tolua_module(tolua_S,NULL,0);
 tolua_beginmodule(tolua_S,NULL);
  tolua_function(tolua_S,"Player__Player",tolua_player_Player__Player00);
  tolua_function(tolua_S,"Player__~Player",tolua_player_Player__~Player00);
  tolua_function(tolua_S,"Player__Run",tolua_player_Player__Run00);
 tolua_endmodule(tolua_S);
 return 1;
}


#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 501
 TOLUA_API int luaopen_player (lua_State* tolua_S) {
 return tolua_player_open(tolua_S);
};
#endif

