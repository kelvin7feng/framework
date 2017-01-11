#ifndef __COMMON_KNETPACKAGE_H__
#define __COMMON_KNETPACKAGE_H__

#include "db_buffer.h"

class IKNetPacket
{
public:
	virtual ~IKNetPacket() {}
	virtual bool GetData(IKG_Buffer** ppBuffer) const = 0;
	virtual bool IsValid() const = 0;
	virtual bool Reset() = 0;
    virtual bool CheckNetPacket(const char* pData, unsigned int uSize) = 0;
	//************************************
	// Method:    Write
	// FullName:  IKNetPackage::Write
	// Access:    virtual public 
	// Returns:   bool true:写入成功 false:写入数据失败
	// Qualifier: 
	// Parameter: const char * pData 写入的数据指针
	// Parameter: unsigned int uDataLen	数据长度
	// Parameter: unsigned int * puWrite 真实写入的长度
	//************************************
	virtual bool Write(const char* pData, unsigned int uDataLen, unsigned int* puWrite) = 0;
};

IKNetPacket* KG_CreateCommonPackage();

#endif
