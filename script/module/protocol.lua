
function OnClientRequest(str_json)
	
    local tb_request = json.decode(str_json);
    LOG_TABLE(tb_request);
	
    return 0;
end

function OnRedisRespone(str_repsonse_json)
	if not IsString(str_repsonse_json) then
		LOG_INFO("response data is nil");
	end

	--local tb_respone = json.decode(str_repsonse_json);
	LOG_INFO(str_repsonse_json);
end