
lua_path = "./../script/?.lua;"
package.path = lua_path .. "common/?.lua;" .. package.path

require("common.class")
require("common.log")
require("common.json")

require("module.protocol")