//
//  ktimer.h
//  thread
//
//  Created by 冯文斌 on 16/12/10.
//  Copyright © 2016年 kelvin. All rights reserved.
//

#ifndef ktimer_h
#define ktimer_h

#include "kmacros.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

class KTimer
{
private:
#ifdef WIN32
    LARGE_INTEGER	m_nFrequency;
    LARGE_INTEGER	m_nTimeStart;
    LARGE_INTEGER	m_nTimeStop;
#else
    timeval m_nTimeStart;
    timeval m_nTimeStop;
#endif
public:
    KTimer();
    void			Start();
    void			Stop();
    unsigned int	GetElapse();
    unsigned long long GetElapseEx();
    unsigned int	GetElapseMicrosecond();
    unsigned int	GetElapseFrequency();
    unsigned int	GetInterval();
    bool			Passed(int nTime);
};

#endif /* ktimer_h */
