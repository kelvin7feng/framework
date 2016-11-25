#pragma once

#include <string>

class FileUtil {
public:
	FileUtil();
	~FileUtil();
	static FileUtil* GetInstance();
	
	bool ReadFile(const char* file_name, std::string& str);
};