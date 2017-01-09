
lua_path = "./../script/?.lua;"
package.path = lua_path .. "common/?.lua;" .. package.path

require("defination.event_def")
require("defination.database_def")
require("defination.error_code_def")

require("common.class")
require("common.log")
require("common.json")
require("common.table")
require("common.type")
require("common.redis")

require("manager.user_manager")

require("module.protocol")

--local player = Player:new();
--player:Run();

LOG_INFO("load logic script succeed...")