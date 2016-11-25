--表复制
function CopyTab(st)
    local tab = {}
    for k, v in pairs(st or {}) do
        if type(v) ~= "table" then
            tab[k] = v
        else
            tab[k] = CopyTab(v)
        end
    end
    return tab
end

-- 合并表
function MergeTab(tFrom, tTo)
	for k,v in pairs(tFrom) do
		tTo[k] = v
	end
end