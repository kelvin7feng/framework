-- 提供一些常用的redis接口

RedisInterface = class()

function RedisInterface:ctor(nType)
	self.m_nRedisType = nType;
	self.m_strRedisTableName = MAP_DATABSE_TABLE_KEY[nType];
end

function RedisInterface:GetRedisTableName()
	return self.m_strRedisTableName;
end

function RedisInterface:GetValue(nUserId, nEventType, strKey)

	if IsNumber(strKey) then
		strKey = tostring(strKey)
	elseif IsTable(strKey) then
		strKey = json.encode(strKey)
	end

	CRedis.PushRedisGet(nUserId, nEventType, self:GetRedisTableName(), strKey);
end

function RedisInterface:SetValue(nUserId, nEventType, strKey, strValue)

	if IsNumber(strValue) then
		strValue = tostring(strValue)
	elseif IsTable(strValue) then
		strValue = json.encode(strValue)
	end
	
	CRedis.PushRedisSet(nUserId, nEventType, self:GetRedisTableName(), strKey, strValue);
end

function RedisInterface:DeleteValue(nUserId, nEventType, strKey)

	if IsNumber(strKey) then
		strKey = tostring(strKey)
	elseif IsTable(strKey) then
		strKey = json.encode(strKey)
	end

	CRedis.PushRedisGet(nUserId, nEventType, self:GetRedisTableName(), strKey);
end

G_AccountRedis = RedisInterface:new(DATABASE_TABLE.ACCCUNT)
