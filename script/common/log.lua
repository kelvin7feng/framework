
--日志打印模块
LOG_TYPE = {
	DEBUG = 3,
	WARN = 2,
	ERROR = 1,
	INFO = 0
}

LOG_HEADER = {
	[LOG_TYPE.DEBUG] = "DEBUG",
	[LOG_TYPE.WARN]  = "WARNING",
	[LOG_TYPE.ERROR] = "ERROR",
	[LOG_TYPE.INFO]  = "INFO"
}

--打印函数
local function LOG_DEBUG_CORE(nLevel, ...)

	local args = {...}
	local strMsg = "Lua: "

	for k,v in ipairs(args) do
		strMsg = strMsg .. tostring(v) .. "\t"
	end

	print(strMsg)
end

--Debug输出
function LOG_DEBUG(...)

	if not DEBUG_SWITCH then return end

	LOG_DEBUG_CORE(LOG_TYPE.DEBUG, ...)
end

--警告输出
function LOG_WARN(...)

	if not DEBUG_SWITCH then return end

	LOG_DEBUG_CORE(LOG_TYPE.WARN, ...)
end

--普通信息输出
function LOG_INFO(...)

	LOG_DEBUG_CORE(LOG_TYPE.INFO, ...)
end

--错误输出
function LOG_ERROR(...)

	LOG_DEBUG_CORE(LOG_TYPE.ERROR, ...)
end

--打印堆栈
function DEBUG_TRACE_BACE(strMessage, nRecursion)
	
	local strError = debug.traceback(strMessage, nRecursion)
	LOG_ERROR(strError)		
end

function OsExit(i)
	os.exit(i)
end

--检查是否为错误
function IsError(value, strMessage)

	local bIsError = not value
	
	if bIsError then
		
		if strMessage then			
			LOG_ERROR(strMessage)		
		else
			DEBUG_TRACE_BACE("@:", 3)
		end
		
	end

	return bIsError
end

--打印table
function LOG_TABLE(value, sub, count, NotFirst, name, ttt)
	if not NotFirst then
	    printCount = 0 --层深统计
	    printMark = {} --重复标记
	    name = name or ""
	    ttt = ttt or ""
	    print("["..os.date("%x %X").."]")
	end

    if type(name) ~= "string" then
		name = tostring(name)
	end
	
	if type(value) == "number" then
		print(ttt..name, "=", value)
	elseif type(value) == "string" then
		print(ttt..name, "=", "\""..value.."\"")
	elseif type(value) == "boolean" or type(value) == "function" or type(value) == "userdata" then
		print(ttt..name, "=", tostring(value))
	elseif type(value) == "table" then
		printCount = printCount + 1
		if not printMark[value] then
			printMark[value] = true
		else
			print(ttt..name, "=", value)
			printCount = printCount - 1
			return
		end
		--print(ttt..name, "=", value)
		if count and printCount > count then
			printCount = printCount - 1
			return
		end
		if sub or (not NotFirst)then
			print(ttt.."{")
			for id, v in pairs(value) do
				LOG_TABLE(v, sub, count, true, id, ttt.."	  ")
			end
			printCount = printCount - 1
			print(ttt.."}")
		end
	else
		print("error print" .. type(value))
	end
end