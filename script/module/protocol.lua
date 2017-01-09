
function OnClientRequest(nEventType, strJson)
	
	LOG_INFO("nEventType:" .. nEventType)
	LOG_INFO("RECEIVE:" .. strJson)
	if not IsNumber(nEventType) then
		LOG_ERROR("event type is not number");
		return 0;
	end

	local tbParam = json.decode(strJson);
	LOG_INFO("EVENT_TYPE.SYSTEM.REGISTER:" .. EVENT_TYPE.SYSTEM.REGISTER);
	if nEventType == EVENT_TYPE.SYSTEM.REGISTER then
		LOG_INFO("CALL REGISTER");
		G_UserManager:Register(tbParam);
	end
	
    return 0;
end

function OnRedisRespone(nUserId, nEventType, strRepsonseJson)
	if not IsString(strRepsonseJson) then
		LOG_INFO("response data is nil");
	end

	LOG_INFO("LUA Redis Response");
	LOG_INFO("nUserId:" .. nUserId);
	LOG_INFO("nEventType:" .. nEventType);
	LOG_INFO("strRepsonseJson:" .. strRepsonseJson);

	if nEventType == EVENT_TYPE.SYSTEM.REGISTER then
		LOG_INFO("register call back...");
	end
end