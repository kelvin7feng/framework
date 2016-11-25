
function ClientRequest(str_json)
	
    str_json = "{\"name\":\"aaa\"}"
    local tb_request = json.decode(str_json)

    LOG_TABLE(tb_request);
    return 0;
end
