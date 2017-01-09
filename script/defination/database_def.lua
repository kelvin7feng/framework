
-- 数据库表名索引
DATABASE_TABLE = 
{
	GLOBAL = 1,
	ACCCUNT = 2,
	GAME_DATA = 3		
}

-- 数据库名字
MAP_DATABSE_TABLE_KEY = 
{
	[DATABASE_TABLE.GLOBAL] 			=  "global",
	[DATABASE_TABLE.ACCCUNT] 			=  "account",
	[DATABASE_TABLE.GAME_DATA] 			=  "game"
}

-- 数据库字段
DATABASE_TABLE_FIELD = 
{
	[DATABASE_TABLE.GLOBAL] 			=  
	{
		USER_ID							=  10000001
	},

	[DATABASE_TABLE.ACCCUNT] 			=  
	{
		USER_INFO						= 
		{
			UserId 						= 0,		-- 玩家Id
			DeviceId					= "",		-- 设备Id
			RegisterIp					= "",		-- 注册Ip
			Platform					= ""		-- 注册渠道	
		}
	},

	[DATABASE_TABLE.GAME_DATA] 			=  
	{
	}
}