--[[
	重定向模块
	--sunyutao
	--2016.06.29
	通过注册的方式将新的模块（接口不全）重定向到旧的模块，
	当调用新模块的函数或字段时，如果新模块找不到相应方法，会返回旧模块的相应函数或字段
]]
local Redirection = {}
Redirection.Redirectionmap = {}


--注册重定向,注意newObj不能是class
function Redirection:RgisterRedirection(newObj,oldObj)
	
	if newObj == nil or oldObj == nil then
		print("Redirection:RgisterRedirection nil",newObj,oldObj)
		return
	end
	
	local mt = getmetatable(newObj)
	if mt == nil then
		mt = {}
	end
	
	if mt.__index ~= nil then
		print("Redirection:RgisterRedirection has Redirection")
		return
	end
	
	mt.__index = function (tb,strIndex)
		
		local result = rawget(tb,strIndex)
		if result ~= nil then
			return result
		end
		
		local oldObj = Redirection.Redirectionmap[newObj]
		
		if oldObj == nil then
			return
		end
		
		result = oldObj[strIndex]
		return result
	end
	
	setmetatable(newObj,mt)
	
	
	Redirection.Redirectionmap[newObj] = oldObj
end


G_Redirection = Redirection
return G_Redirection