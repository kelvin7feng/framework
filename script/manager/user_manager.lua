UserManager = class()

function UserManager:ctor()
	self.m_tbUser = {};
	self.m_nGlobalUserId = 100001;
end

function UserManager:IncrementGlobalUserId()
	self.m_nGlobalUserId = self.m_nGlobalUserId + 1;
end

function UserManager:GetInitUser(tbParam)
	local tbUserInfo = DATABASE_TABLE_FIELD[DATABASE_TABLE.ACCCUNT].USER_INFO;
	tbUserInfo.UserId = self.m_nGlobalUserId;
	tbUserInfo.DeviceId = tbParam.device_id;
	tbUserInfo.RegisterIp = self.ip;
	
	return tbUserInfo;
end

function UserManager:Register(tbParam)
	local nErrorCode = ERROR_CODE.SYSTEM.UNKNOWN_ERROR;
	local strIp = nil;
	local strDeviceId = nil;

	if not IsTable(tbParam) then
		nErrorCode = ERROR_CODE.SYSTEM.PARAMTER_ERROR;
		LOG_ERROR("Register Param of strIp is Error ");

		--goto Exit0;
		return 0;
	end

	strIp = tbParam.ip;
	if not IsString(strIp) then
		nErrorCode = ERROR_CODE.SYSTEM.PARAMTER_ERROR;
		LOG_ERROR("Register Param of strIp is Error ");

		--goto Exit0;
		return 0;
	end

	strDeviceId = tbParam.device_id;
	if not IsString(strDeviceId) then
		nErrorCode = ERROR_CODE.SYSTEM.PARAMTER_ERROR;
		LOG_ERROR("Register Param of strDeviceId is Error ")

		--goto Exit0;
		return 0;
	end

	--to do:检查是否有注册过
	local tbUserInfo = self:GetInitUser(tbParam);
	local nUserId = tbUserInfo.UserId;

	G_AccountRedis:SetValue(nUserId, EVENT_TYPE.SYSTEM.REGISTER, nUserId, tbUserInfo);

	nErrorCode = ERROR_CODE.SYSTEM.OK;
--::Exit0::

	return nErrorCode
end

G_UserManager = UserManager:new()