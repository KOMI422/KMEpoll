#pragma once

#include "KMEpoll.h"
#include <functional>
#include <set>

namespace KM
{

class KMEpollTimer : public KMEpollable
{
public:
    typedef std::function<void(uint64_t nowMs)> EpollTimerCb;

    struct EpollTimerContext
    {
        uint64_t intervalMs;
        uint64_t lastTriggerTs;
        EpollTimerCb timerCb;

        EpollTimerContext() : intervalMs(0), lastTriggerTs(0) {}
    };
    typedef std::shared_ptr<EpollTimerContext> EpollTimerContext_Ptr;

    static uint64_t getNowMs();
public:
    KMEpollTimer();
    virtual ~KMEpollTimer();

    virtual void onEpollEvents(uint32_t evts, uint32_t timerMs);
    virtual int32_t getEpollableFd() const { return m_timerFd; }
    virtual bool isWaitingReadEvent() const { return (m_timerContexts.size() > 0); }
    virtual bool isWaitingWriteEvent() const { return false; }

    void closeTimer();
    EpollTimerContext_Ptr addTimerCallback(uint32_t intervalMs, EpollTimerCb cb);
    void removeTimerCallback(EpollTimerContext_Ptr pCtx);
private:
    static const uint32_t DEFAULT_TIMER_INTERVAL_MS;
private:
    int32_t m_timerFd;
    std::set<EpollTimerContext_Ptr> m_timerContexts;
};

}; // namespace KM