-- huangxiaoming@kingsoft.com

-- 提供一些常用的redis接口

-- 迭代器，用于循环取值
RedisValueSet = class()

function RedisValueSet:ctor(nCacheId, nSize)
	--KDebug.ProcessError(nCacheId >= 0)
	--KDebug.ProcessError(nSize >= 0)
	self.m_nSize = nSize
	self.m_nCurPos = 0
	self.m_nCacheId = nCacheId
end

function RedisValueSet:Size()
	return self.m_nSize
end

-- 设置游标索引
function RedisValueSet:Seek(nPos)
	nPos = nPos or 0
	--KDebug.ProcessError(nPos > 0)
	self.m_nCurPos = 0
end

-- 取出下一个值
function RedisValueSet:Next()
	if self.m_nCurPos >= self.m_nSize then
		return nil
	end
	self.m_nCurPos = self.m_nCurPos + 1
	local bRet, strKey, strValue = ScriptGetGetAllCache(self.m_nCacheId, self.m_nCurPos)
	if IsError(bRet) then
		return nil
	end
	return strKey, strValue
end

-- 释放缓存，使失效,用完了一定要释放
function RedisValueSet:Invalid()
	if self.m_nCacheId <= 0 then
		return
	end
	ScriptDeleteGetAllCache(self.m_nCacheId)
	self.m_nCacheId = 0
	self.m_nSize = 0
end

RedisInterface = class()

function RedisInterface:ctor(nType)
	--KDebug.ProcessError(nType)
	self.m_nRedisType = nType
end

-- 获取锁，false表示该数据锁正在被使用,与releaseLock配对使用
function RedisInterface:GetLock(strKey, nTimeOut)
	local bResult = false
	local strLockKey = nil
	local nMutexValue = 0
	local strTmp = nil
	local bRet = false
	local nLastLockTime = nil
	local nLockTime = nil
	local nCurTime = os.time()
	nTimeOut = nTimeOut or 5
	if not IsString(strKey) then
		goto Exit0
	end
	if not IsNumber(nTimeOut) then
		goto Exit0
	end
	nTimeOut = math.ceil(nTimeOut)
	strLockKey = string.format("SCRIPT_MUTEX_%s", strKey)
	bRet = ScriptSetNoExistValue(self.m_nRedisType, strKey, tostring(nCurTime))
	if bRet == true then
		LOG_DEBUG("RedisInterface:GetLock success normal", strKey, nTimeOut)
		goto Exit1
	end
	if nTimeOut <= 0 then
		LOG_DEBUG("RedisInterface:GetLock fail normal", strKey, nTimeOut)
		goto Exit1
	end
	strTmp = ScriptGetValue(self.m_nRedisType, strKey)
	nLastLockTime = tonumber(strTmp)
	if not IsNumber(nLastLockTime) then
		goto Exit0
	end
	if nCurTime - nLastLockTime < nTimeOut then
		LOG_DEBUG("RedisInterface:GetLock fail non-timeout", strKey, nTimeOut, nCurTime, nLastLockTime)
		goto Exit0
	end
	strTmp = ScriptGetSetValue(self.m_nRedisType, strKey, tostring(nCurTime))
	nLockTime = tonumber(strTmp)
	if nLockTime ~= nLastLockTime then	-- 被别人抢先了
		LOG_DEBUG("RedisInterface:GetLock fail other first", strKey, nTimeOut, nCurTime, nLastLockTime, nLockTime)
		goto Exit0
	end
	LOG_DEBUG("RedisInterface:GetLock success timeout", strKey, nTimeOut, nCurTime, nLastLockTime)
::Exit1::
	bResult = true
::Exit0::
	return bResult
end

-- 释放锁
function RedisInterface:ReleaseLock(strKey)
	return self:DeleteValue(strKey)
end

-- 锁数据，会阻塞等待超时
function RedisInterface:Lock(strKey, nTimeOut)
	local bRet = false
	local nTryNum = 0
	while (nTryNum < 60) do
		bRet = self:GetLock(strKey, nTimeOut)
		if bRet then
			LOG_INFO("RedisInterface:Lock success", strKey, nTryNum)
			break
		end
		LuaSleep(1)
		nTryNum = nTryNum + 1
		LOG_INFO("RedisInterface:Lock Try Num:", strKey, nTryNum)
	end
	return bRet
end

-- 解除锁
function RedisInterface:UnLock(strKey)
	return self:DeleteValue(strKey)
end


function RedisInterface:GetValue(strKey)
	--KDebug.ProcessNotString(strKey)
	return ScriptGetValue(self.m_nRedisType, strKey)
end

function RedisInterface:SetValue(strKey, strValue)
	--KDebug.ProcessNotString(strKey)
	
	local strType = type(strValue)
	if strType == "number" then
		strValue = tostring(strValue)
	elseif strType == "table" then
		strValue = cjson.encode(strValue)
	end
	
	--KDebug.ProcessNotString(strValue)
	return ScriptSetValue(self.m_nRedisType, strKey, strValue)
end

-- 压缩数据接口，针对存储比较大，数量比较多的字符串
function RedisInterface:GetCompressValue(strKey)
	--KDebug.ProcessNotString(strKey)
	return ScriptGetCompressValue(self.m_nRedisType, strKey)
end

function RedisInterface:SetCompressValue(strKey, strValue)
	--KDebug.ProcessNotString(strKey)
	
	local strType = type(strValue)
	if strType == "number" then
		strValue = tostring(strValue)
	elseif strType == "table" then
		strValue = cjson.encode(strValue)
	end
	
	--KDebug.ProcessNotString(strValue)
	return ScriptSetCompressValue(self.m_nRedisType, strKey, strValue)
end 

function RedisInterface:IncreaseValue(strKey, nIncrease)
	--KDebug.ProcessNotString(strKey)
	--KDebug.ProcessNotNumber(nIncrease)
	return ScriptIncreaseValue(self.m_nRedisType, strKey, nIncrease)
end

function RedisInterface:DeleteValue(strKey)
	--KDebug.ProcessNotString(strKey)
	return ScriptDeleteValue(self.m_nRedisType, strKey)
end

function RedisInterface:GetHashValue(strTableName, strKey)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	local strValue = ScriptGetHashValue(self.m_nRedisType, strTableName, strKey)
	LOG_DEBUG("RedisInterface:GetHashValue:", strTableName, strKey, strValue)
	return strValue
end

function RedisInterface:SetHashValue(strTableName, strKey, strValue)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	
	local strType = type(strValue)
	if strType == "number" then
		strValue = tostring(strValue)
	elseif strType == "table" then
		strValue = cjson.encode(strValue)
	end
	
	--KDebug.ProcessNotString(strValue)
	LOG_DEBUG("RedisInterface:SetHashValue", strTableName, strKey)
	return ScriptSetHashValue(self.m_nRedisType, strTableName, strKey, strValue)
end

function RedisInterface:GetCompressHashValue(strTableName, strKey)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	local strValue = ScriptGetCompressHashValue(self.m_nRedisType, strTableName, strKey)
	LOG_DEBUG("RedisInterface:GetCompressHashValue:", strTableName, strKey)
	return strValue
end

function RedisInterface:SetCompressHashValue(strTableName, strKey, strValue)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	--KDebug.ProcessNotString(strValue)
	LOG_DEBUG("RedisInterface:SetCompressHashValue", strTableName, strKey)
	return ScriptSetCompressHashValue(self.m_nRedisType, strTableName, strKey, strValue)
end

function RedisInterface:IncreaseHashValue(strTableName, strKey, nIncrease)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	--KDebug.ProcessNotNumber(nIncrease)
	LOG_DEBUG("RedisInterface:IncreaseHashValue", strTableName, strKey, nIncrease)
	return ScriptIncreaseHashValue(self.m_nRedisType, strTableName, strKey, nIncrease)
end

function RedisInterface:DeleteHashValue(strTableName, strKey)
	--KDebug.ProcessNotString(strTableName)
	--KDebug.ProcessNotString(strKey)
	LOG_DEBUG("RedisInterface:DeleteHashValue", strTableName, strKey)
	return ScriptDeleteHashValue(self.m_nRedisType, strTableName, strKey)
end

-- 处理数据量很大的表
function RedisInterface:HashGetAll(strTableName)
	--KDebug.ProcessNotString(strTableName)
	local nCacheId, nSize = ScriptGetHashAll(self.m_nRedisType, strTableName)
	if nCacheId < 0 or nSize < 0 then
		LOG_ERROR("RedisInterface:HashGetAll error", strTableName, nCacheId, nSize)
		return nil
	end
	LOG_DEBUG("RedisInterface:HashGetAll", "CacheId:" .. nCacheId, "Size" .. nSize)
	return RedisValueSet:new(nCacheId, nSize)
end

-- 处理数据量比较小的表，返回一个table，大数据量用HashGetAll
function RedisInterface:HashGetAllEx(strTableName)
	--KDebug.ProcessNotString(strTableName)
	local tb = ScriptGetHashAllEx(self.m_nRedisType, strTableName)
	--KDebug.ProcessNotTable(tb)
	LOG_DEBUG("RedisInterface:HashGetAllEx:", strTableName, cjson.encode(tb))
	return tb
end

G_ActivityRedis = RedisInterface:new(1)
G_UserRedis = RedisInterface:new(3)
