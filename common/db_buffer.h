//
//  db_buffer.h
//  thread
//
//  Created by 冯文斌 on 16/12/7.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef db_buffer_h
#define db_buffer_h

struct IKG_Buffer
{
public:
    virtual long AddRef() = 0;
    virtual long Release() = 0;
    virtual void* GetData()= 0;
    virtual const void* GetData() const = 0;
    virtual unsigned int GetSize() = 0;
    virtual unsigned int GetReserveSize() = 0;
    virtual void* GetReserveData() = 0;
};

IKG_Buffer* DB_MemoryCreateBuffer(unsigned int uSize);

IKG_Buffer* DB_CreateRedisExpireBuffer(const std::string& szKey, int nSecond);

IKG_Buffer* DB_CreateSetBuffer(const std::string& szTable, const std::string& szKey, const std::string& szValue);

IKG_Buffer* DB_CreateGetBuffer(const std::string& szTable, const std::string& szKey);

IKG_Buffer* DB_CreateHsetBuffer(const std::string& szTable, const std::string& szKey, const std::string& szValue);

IKG_Buffer* DB_CreateHgetBuffer(const std::string& szTable, const std::string& szKey);

IKG_Buffer* DB_CreateDelBuffer(const std::string& szTable, const std::string& szKey);

IKG_Buffer* DB_CreateHdelBuffer(const std::string& szTable, const std::string& szKey);

#endif /* db_buffer_h */
