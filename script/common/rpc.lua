
-- RPC模块
RPC = {
	-- 保存此次会话的信息，这个对应go的KRpcLua结构。	
	--[[
	type KRpcLua struct {
		// IP
		IP string
		// 模块
		Module int32
		// 函数
		Function string
		// UID 用于DataCenter
		UID int64
		// SessionID 用于Gateway
		Session string
		// 数据
		Data string
		// 用于回调
		CallBack string
	}
	--]]
	SessionKRpcLua = {},
	
	HasAnswerClient = false	
	
}

-- 用户UID  原来残留的全局函数，先留着，等下优化的时候再干掉。
USER_UID = 0
SERVER_ID = 0


-- 设置当前对话用户的UID,仅测试用
function RPC.SetUIDForTest(nUID)

	local bResult = false

	if KDebug.SingleArgIsNil(nUID) then
		goto Exit0
	end

	if not RPC.SessionKRpcLua then
		RPC.SessionKRpcLua = {}
	end

	RPC.SessionKRpcLua.UID = nUID
	
	bResult = true
::Exit0::
	return bResult,  nUID
end

-- 获取当前对话的用户IP
function RPC.GetUserIP()	
		
	local strUserIP
	local bResult = false
	
	if IsNil(RPC.SessionKRpcLua, 0) then
		goto Exit0
	end
	
	if IsNil(RPC.SessionKRpcLua.strUserIP, 0) then
		goto Exit0
	end
		
	strUserIP = RPC.SessionKRpcLua.strUserIP
	
	bResult = true
::Exit0::
	return bResult,  strUserIP
	
end

-- 获取当前对话用户的UID
function RPC.GetUID()

	local nUID = 0
	local bResult = false
	
	if IsNil(RPC.SessionKRpcLua, 0) then
		goto Exit0
	end
	
	if IsNil(RPC.SessionKRpcLua.UID, 0) then
		goto Exit0
	end
	
	nUID = RPC.SessionKRpcLua.UID
	
	bResult = true
::Exit0::
	return bResult,  nUID
end


function RPC.Reset()
	
	RPC.SessionKRpcLua = {}
	RPC.HasAnswerClient = false
end

-- 调用其他服务器的lua函数
function CallServer(nModule, strObj, strFunction, ...)

	if type(strFunction) ~= "string" then
		return
	end

    local arg = {...}
	if #arg == 0 then

		t = CallServerLua(nModule, strObj, strFunction, "")

	else

		t = CallServerLua(nModule, strObj, strFunction, cjson.encode(arg))

	end

	if t == nil  or t == "" then

		return
	else

		rt = cjson.decode(t)

		return RPC.tableUnpack(rt)

	end

end

--[[ 
暂时没有这个需求，先注释。
注意这里如果需要实现，需要支持表函数。

-- 异步调用其他服务器的lua函数
function CallServerAsyn(nModule, strFunction, callback ,...)
    local arg = {...}
	if callback == nil then
		callback = ""
	end

	if #arg == 0 then

		CallServerLuaAsyn(nModule, strFunction, "",callback)

	else

		CallServerLuaAsyn(nModule, strFunction, cjson.encode(arg),callback)

	end

end
--]]

-- 调用信息中心服务器
function CallDataCenter(nUID, strObj, strFunction, ...)

	local arg = {...}

	if #arg == 0 then
		t = CallDataCenterLua(nUID, strObj, strFunction, "")

	else
		strJson = cjson.encode(arg)
		t = CallDataCenterLua(nUID, strObj, strFunction, strJson)

	end


	if t == nil or t == "" then
		return

	else

		rt = cjson.decode(t)

		return RPC.tableUnpack(rt)


	end

end

-- 调用日志服务器
function CallLogServer(nUID, strObj, strFunction, ...)

	local arg = {...}

	if #arg == 0 then

		t = CallLogServerGo(nUID, strObj, strFunction, "")

	else

		strJson = cjson.encode(arg)

		t = CallLogServerGo(nUID,  strObj, strFunction, strJson)

	end

		if t == nil or t == "" then

		return

	else

		rt = cjson.decode(t)

		--return TableUnpack(rt)
		return rt
	end

end





function CallLocalWithUid(strObj, strFunction, args,uid)
	local bResult = false
    local tResultDataList = nil
	local bRetCode = nil
	

    USER_UID = uid
	if GetServerId then
		local serverID = GetServerId()
		if not serverID then
			LOG_ERROR("GetServerId faild ")
		else
			SERVER_ID = serverID
		end	
	end
	
	
	bRetCode,tResultDataList = G_MessageManager:CallFromClient(strObj,strFunction,args)
	--strRetData, bResult = CallLocal(strObj, strFunction, args)


	return strRetData
end

-- 客户端调用本地函数，
-- !!! 注意 该函数仅供go调用，lua请勿调用。

function CallLocalFromClient(inRpcLua)

	LOG_DEBUG("CallLocalFromClient,inRpcLua")
	
	local bRetCode = false
	local tResultDataList = nil
	local args = nil
	
	RPC.SessionKRpcLua = inRpcLua
	USER_UID = inRpcLua.UID
	SERVER_ID = inRpcLua.ServerID
	
	LOG_DEBUG("CallLocalFromClient,inRpcLua",USER_UID,SERVER_ID)
	
	--RPC.HasCallClient = false
	
	if inRpcLua.Data and inRpcLua.Data ~= "" then
		args = cjson.decode(inRpcLua.Data)
	end

	bRetCode, tResultDataList = G_MessageManager:CallFromClient(inRpcLua.Object, inRpcLua.Function, RPC.tableUnpack(args))
	
	--调用失败
	if bRetCode ~= true then
		AnswerClient({-1,{},inRpcLua.Object,inRpcLua.Function})
	else
		AnswerClient(tResultDataList)
	end
	
	--
	RPC.SessionKRpcLua = {}
	RPC.HasAnswerClient = false
	
	return ""
end



--回应客户端
--参数tResultDataList：ErrCode,...
--若ErrCode == 0 ,...：EventList,callbackObj，callbackFunc,args
function AnswerClient(tResultDataList)
	
	if RPC.HasAnswerClient then
		--DEBUG_TRACE_BACE()
		return 
	end
	
	
	if RPC.SessionKRpcLua.SerialID == nil then
		--DEBUG_TRACE_BACE()
		return 
	end

	if #tResultDataList > 0 then
		strArgs = cjson.encode(tResultDataList)
		strArgs = strArgs or ""
	end
	
	CallClientLua(USER_UID,"g_CUIGameRPCManager","OnReciveResponse", strArgs or "",RPC.SessionKRpcLua.SerialID)
	
	
	RPC.HasAnswerClient = true
end


--
function CallClient(uid, strObj, strFunction, ...)
	local args = {...}
	local strArgs = ""
	if #arg > 0 then
		strArgs = cjson.encode(arg)
		strArgs = strArgs or ""
	end
	
	
	CallClientLua(uid, strObj, strFunction, strArgs,0)
end

--批量推送消息到客户端
function CallClientBatchLua(uidList, strObj, strFunction, ...)
	if G_IS_DISABLE_PUSH == true then
		return
	end
	
	if KDebug.ProcessNotTable(uidList) then
		return
	end
	
	if #uidList <= 0 then
		LOG_WARN("CallClientBatch","#uidList == 0")
		return
	end
	
	if SERVER_ID == nil or SERVER_ID == 0 then
		SERVER_ID = GetServerId()
	end
	
	local targetList = {}
	for index,uid in ipairs(uidList) do
		local target = {}
		target.UID = uid
		target.ServerID = SERVER_ID
		table.insert(targetList,target)
	end
	
	local arg = cjson.encode({...})
	if arg == nil then
		arg = ""
	end
	
	KDebug.PrintDebug("CallClientBatchLua>>>>",targetList)
	CallClientBatch(targetList, strObj, strFunction, arg, 0)
	
end



--推送消息给在线用户
function CallServerAllLua(strObj, strFunction, ...)
	if G_IS_DISABLE_PUSH == true then
		return
	end
	
	local arg = cjson.encode({...})
	if arg == nil then
		arg = ""
	end
	CallServerAllClient(SERVER_ID,strObj,strFunction,arg)
end


function CallServerClientLua(nServerID, strObj, strFunction, ...)
	if KDebug.ProcessNotNumber(nServerID) then
		return
	end
	
	if SERVER_ID == nil or SERVER_ID == 0 then
		SERVER_ID = GetServerId()
	end
	
	local arg = cjson.encode({...})
	if arg == nil then
		arg = ""
	end
	
	KDebug.PrintDebug("CallServerClientLua>>>>",nServerID,strObj, strFunction, ...)
	CallServerClient(nServerID, strObj, strFunction, arg)
end

------------------------------------------------------------------------------------------------------------
-- 私有函数

function RPC.jsonDecode(strJson)
	
	if KDebug.ProcessNotString(strJson) then
		return nil
	end
	
	t = cjson.decode(strJson)
		
end

function RPC.tableUnpack(t)
	
	if t == nil then
		return
	end

	-- 这里有时候会是 数值的索引，有时候是 字符串"1"索引，所以必须做处理。鬼知道为什么会这样
	if t[1] ~= nil then

		return unpack(t)

	else

		tResult = {}
		
		local i = 1
		
		while i <= 9999 do
		
			if t[tostring(i)] == nil then

				break

			end

			tResult[i] = t[tostring(i)]
		
			i = i + 1
		end
						
		return unpack(tResult)

	end

end




