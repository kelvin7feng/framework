#include "ktimer.h"

#ifndef WIN32
#include <sys/time.h>
#endif

KTimer::KTimer()
{
#ifdef WIN32
	m_nFrequency.QuadPart = 200 * 1024 * 1024;
	m_nTimeStart.QuadPart = 0;
	m_nTimeStop.QuadPart = 0;
	QueryPerformanceFrequency(&m_nFrequency);
#else
	//m_nFrequency = CLOCKS_PER_SEC;
#endif
}
//---------------------------------------------------------------------------
// 函数:	Start
// 功能:	开始计时
//---------------------------------------------------------------------------
void KTimer::Start()
{
#ifdef WIN32
	QueryPerformanceCounter(&m_nTimeStart);
#else
	gettimeofday(&m_nTimeStart, NULL);
#endif
}
//---------------------------------------------------------------------------
// 函数:	Stop
// 功能:	停止计时
//---------------------------------------------------------------------------
void KTimer::Stop()
{
#ifdef WIN32
	QueryPerformanceCounter(&m_nTimeStop);
#else
	gettimeofday(&m_nTimeStop, NULL);
#endif
}
//---------------------------------------------------------------------------
// 函数:	GetElapse
// 功能:	计算从开始计时到现在已经过到时间
// 返回:	unsigned int in ms
//---------------------------------------------------------------------------
unsigned int KTimer::GetElapse()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (unsigned int)((nTime.QuadPart - m_nTimeStart.QuadPart) 
		* 1000 / m_nFrequency.QuadPart);
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - m_nTimeStart.tv_sec) * 1000 + (tv.tv_usec - m_nTimeStart.tv_usec) / 1000;
#endif
}

unsigned long long KTimer::GetElapseEx()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (nTime.QuadPart - m_nTimeStart.QuadPart) 
		* 1000 / m_nFrequency.QuadPart;
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - m_nTimeStart.tv_sec) * 1000 + (tv.tv_usec - m_nTimeStart.tv_usec) / 1000;
#endif
}

//---------------------------------------------------------------------------
// 函数:	GetElapseFrequency
// 功能:	计算从开始计时到现在已经过到时间
// 返回:	unsigned int in frequency
//---------------------------------------------------------------------------
unsigned int KTimer::GetElapseFrequency()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (unsigned int)(nTime.QuadPart - m_nTimeStart.QuadPart);
#endif
	return 0;
}
//---------------------------------------------------------------------------
// 函数:	GetElapseFrequency
// 功能:	计算从开始计时到现在已经过到时间
// 返回:	unsigned int in frequency
//---------------------------------------------------------------------------
unsigned int KTimer::GetElapseMicrosecond()
{
#ifdef WIN32
	LARGE_INTEGER nTime;
	QueryPerformanceCounter(&nTime);
	return (unsigned int)((nTime.QuadPart - m_nTimeStart.QuadPart) 
		* 1000000 / m_nFrequency.QuadPart);
#else
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec - m_nTimeStart.tv_sec) * 1000000 + tv.tv_usec - m_nTimeStart.tv_usec;
#endif
}
//---------------------------------------------------------------------------
// 函数:	GetInterval
// 功能:	取得从开始到停止之间的时间间隔，以毫秒为单位
// 返回:	毫秒值
//---------------------------------------------------------------------------
unsigned int KTimer::GetInterval()
{
#ifdef WIN32
	return (unsigned int)((m_nTimeStop.QuadPart - m_nTimeStart.QuadPart) 
		* 1000 / m_nFrequency.QuadPart);
#else
	return (m_nTimeStop.tv_sec - m_nTimeStart.tv_sec) * 1000 + (m_nTimeStop.tv_usec - m_nTimeStart.tv_usec) / 1000;
#endif
}

//---------------------------------------------------------------------------
// 函数:	Passed
// 功能:	看是否过了nTime毫秒
// 参数:	nTime	毫秒
// 返回:	true	已经过了
//			false	还没有过
//---------------------------------------------------------------------------
bool KTimer::Passed(int nTime)
{

	if (GetElapse() >= (unsigned int)nTime)
	{
		Start();
		return true;
	}
	return false;
}

