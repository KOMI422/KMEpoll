#include "KMEpollTimer.h"
#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <sys/time.h>

using namespace KM;

const uint32_t KMEpollTimer::DEFAULT_TIMER_INTERVAL_MS = 100;

uint64_t KMEpollTimer::getNowMs()
{
    struct timeval nowTs;
    gettimeofday(&nowTs, NULL);

    return (nowTs.tv_sec * 1000 + nowTs.tv_usec / 1000);
}

KMEpollTimer::KMEpollTimer()
{
    m_timerFd = -1;
    m_timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}

KMEpollTimer::~KMEpollTimer()
{
    if (m_timerFd != -1)
    {
        ::close(m_timerFd);
    }
}

void KMEpollTimer::onEpollEvents(uint32_t evts)
{
    int32_t errNo = 0;
    if (KMEpollable::isError(evts, errNo))
    {
        throw KMEpollException("KMEpollTimer read exception msg=" + std::string(strerror(errNo)));
    }

    if (!KMEpollable::isReadable(evts))
    {
        return;
    }

    uint64_t value = 0;
    if (read(m_timerFd, &value, sizeof(uint64_t)) > 0 && value > 0)
    {
        uint64_t nowMs = KMEpollTimer::getNowMs();
        for(EpollTimerContext_Ptr pCtx : m_timerContexts)
        {
            if(pCtx->lastTriggerTs == 0)
            {
                pCtx->lastTriggerTs = nowMs;
                continue;
            }

            if(pCtx->lastTriggerTs + pCtx->intervalMs > nowMs)
            {
                continue;
            }

            pCtx->lastTriggerTs = nowMs;
            if(pCtx->timerCb)
            {
                pCtx->timerCb(nowMs);
            }
        }
    }
}

void KMEpollTimer::closeTimer()
{
    ::close(m_timerFd);
    m_timerFd = -1;
}

KMEpollTimer::EpollTimerContext_Ptr KMEpollTimer::addTimerCallback(uint32_t intervalMs, EpollTimerCb cb)
{
    if(!cb)
    {
        throw KMEpollException("KMEpollTimer::addTimerCallback invalid timerCallback");
    }

    EpollTimerContext_Ptr pCtx = std::make_shared<EpollTimerContext>();
    pCtx->intervalMs = intervalMs;
    pCtx->lastTriggerTs = 0;
    pCtx->timerCb = cb;
    m_timerContexts.insert(pCtx);

    if(m_timerContexts.size() == 1)
    {
        struct itimerspec timerInterval;
        timerInterval.it_interval.tv_sec = timerInterval.it_value.tv_sec = 0;
        timerInterval.it_interval.tv_nsec = timerInterval.it_value.tv_nsec = DEFAULT_TIMER_INTERVAL_MS * 1000 * 1000;
        timerfd_settime(m_timerFd, 0, &timerInterval, NULL);
    }
}

void KMEpollTimer::removeTimerCallback(EpollTimerContext_Ptr pCtx)
{
    m_timerContexts.erase(pCtx);

    if(m_timerContexts.empty())
    {
        struct itimerspec timerInterval;
        timerInterval.it_interval.tv_sec = 0;
        timerInterval.it_interval.tv_nsec = 0;
        timerfd_settime(m_timerFd, 0, &timerInterval, NULL);
    }
}