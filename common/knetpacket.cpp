#include "kmacros.h"
#include "knetpacket.h"
#include "google.pb.h"

#include <memory.h>

#define KD_PACKAGE_LEN_SIZE sizeof(unsigned int)
#define KD_INVALID_PACKET_LEN (unsigned int)-1

using namespace google;

class KNetPackage : public IKNetPacket
{
public:
	KNetPackage();
	virtual ~KNetPackage();
	virtual bool GetData(IKG_Buffer** ppBuffer) const;
	virtual bool Write(const char* pData, unsigned int uDataLen, unsigned int* puWrite);
	virtual bool IsValid() const { return m_uRecvOffset == m_uPacketLen; }
	virtual bool Reset();
    virtual bool CheckNetPacket(const char* pData, unsigned int uSize);
private:
	bool _InitPackage(unsigned int uPackageSize);
	bool _WriteData(const char* pData, unsigned int uSize);
private:
	char m_szRemain[KD_PACKAGE_LEN_SIZE];	
	unsigned int m_uPacketLen;
	unsigned int m_uRecvOffset;
	IKG_Buffer* m_pBuffer;
};

IKNetPacket* KG_CreateCommonPackage()
{
	IKNetPacket* pPackage = new KNetPackage;
	return pPackage;
}

KNetPackage::KNetPackage() :
	m_uPacketLen(KD_INVALID_PACKET_LEN),
	m_uRecvOffset(0),
	m_pBuffer(NULL)
{
	memset(m_szRemain, 0, sizeof(m_szRemain));
}

KNetPackage::~KNetPackage()
{
	SAFE_RELEASE(m_pBuffer);
}

bool KNetPackage::GetData(IKG_Buffer** ppBuffer) const
{
	bool bResult = false;
	//KGLOG_PROCESS_ERROR(ppBuffer);
	//KG_PROCESS_ERROR(IsValid());
	//KGLOG_PROCESS_ERROR(m_pBuffer);
    if(!IsValid())
        goto Exit0;
	*ppBuffer = m_pBuffer;
	bResult = true;
Exit0:
	return bResult;
}

bool KNetPackage::Write(const char* pData, unsigned int uDataLen, unsigned int* puWrite)
{
	bool bResult = false;
	bool bRet = false;
	int nRet = 0;
	unsigned int uPackageLen = 0;
	unsigned int uMaxWriteLen = 0;
	unsigned int uCurWriteLen = 0;
	//KG_PROCESS_SUCCESS(m_uPacketLen == m_uRecvOffset);
    if(m_uPacketLen == m_uRecvOffset)
        goto Exit1;
	//KGLOG_PROCESS_ERROR(puWrite);
	*puWrite = 0;
	//KGLOG_PROCESS_ERROR(pData);
	//KG_PROCESS_SUCCESS(uDataLen == 0);
    
    //如果没有接够头的长度，需要先把长度读取出来
	if (m_uRecvOffset < KD_PACKAGE_LEN_SIZE)
	{
		for (int i = m_uRecvOffset; i < KD_PACKAGE_LEN_SIZE; ++i)
		{
			m_szRemain[m_uRecvOffset++] = *pData++;
			++uCurWriteLen;
			if (m_uRecvOffset == KD_PACKAGE_LEN_SIZE)
			{
				bRet = _InitPackage(*(unsigned int*)m_szRemain);
				//KGLOG_PROCESS_ERROR(bRet);
			}
			//KG_PROCESS_SUCCESS(uCurWriteLen == uDataLen);
            if(uCurWriteLen == uDataLen)
                goto Exit1;
		}
		//KG_PROCESS_SUCCESS(uCurWriteLen == uDataLen);
        if(uCurWriteLen == uDataLen)
            goto Exit1;
	}
	if (m_uRecvOffset < m_uPacketLen)
	{
		uMaxWriteLen = uDataLen - uCurWriteLen;
		uMaxWriteLen = uMaxWriteLen < (m_uPacketLen - m_uRecvOffset) ? uMaxWriteLen : (m_uPacketLen - m_uRecvOffset);
		//KGLOG_PROCESS_ERROR(_WriteData(pData, uMaxWriteLen));
        if(!_WriteData(pData, uMaxWriteLen))
            goto Exit1;
		uCurWriteLen += uMaxWriteLen;
	}
Exit1:
	*puWrite = uCurWriteLen;
	bResult = true;
Exit0:
	return bResult;
}

bool KNetPackage::Reset()
{
	m_uPacketLen = KD_INVALID_PACKET_LEN;
	m_uRecvOffset = 0;
	SAFE_RELEASE(m_pBuffer);
	return true;
}

bool KNetPackage::_InitPackage(unsigned int uPacketSize)
{
	bool bResult = false;
	//KGLOG_PROCESS_ERROR(m_pBuffer == NULL);
	//KGLOG_PROCESS_ERROR(uPacketSize > KD_PACKAGE_LEN_SIZE);
	//KGLOG_PROCESS_ERROR(KD_INVALID_PACKET_LEN != uPacketSize);
	m_uPacketLen = uPacketSize;
	m_pBuffer = DB_MemoryCreateBuffer(uPacketSize - KD_PACKAGE_LEN_SIZE);
	//KGLOG_PROCESS_ERROR(m_pBuffer);
	bResult = true;
Exit0:
	return bResult;
}

bool KNetPackage::_WriteData(const char* pData, unsigned int uSize)
{
	bool bResult = false;
	//KGLOG_PROCESS_ERROR(pData);
	//KGLOG_PROCESS_ERROR(uSize > 0);
	//KGLOG_PROCESS_ERROR(m_uRecvOffset + uSize <= m_uPacketLen);
	//KGLOG_PROCESS_ERROR(m_pBuffer);
	memcpy((char *)m_pBuffer->GetData() + (m_uRecvOffset - KD_PACKAGE_LEN_SIZE), pData, uSize);
	m_uRecvOffset += uSize;
	bResult = true;
Exit0:
	return bResult;
}

bool KNetPackage::CheckNetPacket(const char* pData, unsigned int uSize)
{
    bool bResult = false;
    Message msg;
    if(msg.ParseFromArray(pData + KD_PACKAGE_LEN_SIZE, uSize))
    {
        bResult = true;
    }
    //测试
    bResult = true;
    return bResult;
}