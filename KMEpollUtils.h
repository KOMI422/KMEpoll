#pragma once

#include <stdint.h>

namespace KM
{

class KMEpollUtils
{
public:
    static uint64_t getNowMs();
};

template<typename T>
class KMTimeChecker
{
public:
    KMTimeChecker() : m_lastCheckTime(0), m_checkInterval(0) {}
    virtual ~KMTimeChecker() {}

    void setInterval(T interval) 
    {
        m_checkInterval = interval;
    }

    bool timeCheck(T now)
    {
        if(m_checkInterval == 0)
            return false;
        
        if(m_lastCheckTime == 0)
        {
            m_lastCheckTime = now;
            return false;
        }
        
        if(m_lastCheckTime + m_checkInterval > now)
        {
            return false;
        }
        m_lastCheckTime = now;
        return true;
    }
private:
    T m_lastCheckTime;
    T m_checkInterval;
};

};