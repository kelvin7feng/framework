//
//  file_util.h
//  server
//
//  Created by 冯文斌 on 16/11/14.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef file_util_h
#define file_util_h

#include <string>

class FileUtil {
public:
	FileUtil();
	~FileUtil();
	static FileUtil* GetInstance();
	
	bool ReadFile(const char* file_name, std::string& str);
};

#endif /* file_util_h */