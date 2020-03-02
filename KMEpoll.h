#pragma once
#include <sys/epoll.h>
#include <memory>
#include <atomic>
#include <set>

namespace KM
{

class KMEpollException : public std::exception
{
public:
    KMEpollException(const std::string& exceptionMsg = "") : m_exceptionMsg(exceptionMsg) {}
    virtual const char* what() const noexcept { return m_exceptionMsg.c_str(); }
private:
    const std::string m_exceptionMsg;
};

class KMEpollable
{
public:
    static bool isReadable(uint32_t evts);
    static bool isWritable(uint32_t evts);
    static bool isError(uint32_t evts, int32_t& errNo);
public:
    KMEpollable() : m_isEdgeTrigger(false), m_canDestroy(false) {}
    virtual ~KMEpollable() {}

    void setEdgeTrigger(bool et) { m_isEdgeTrigger = et; }
    bool isEdgeTrigger() const { return m_isEdgeTrigger; }

    void setCanDestroy(bool val) { m_canDestroy = val; }
    bool canDestroy() const { return m_canDestroy; };
public:
    virtual void onEpollEvents(uint32_t evts) = 0;
    virtual int32_t getEpollableFd() const = 0;
    virtual bool isWaitingReadEvent() const = 0;
    virtual bool isWaitingWriteEvent() const = 0;
    virtual bool checkTimeOut(uint64_t nowMs) = 0;
private:
    bool m_isEdgeTrigger;
    bool m_canDestroy;
};

class KMEpoll
{
public:
    KMEpoll();
    ~KMEpoll();

    void runEpoll();
    void stopEpoll() { m_running = false; }

    void registerEpollable(std::shared_ptr<KMEpollable> pEpollablePtr);
private:
    void setEpollCtrl(std::shared_ptr<KMEpollable> pEpollable, int32_t op, uint32_t evts, void* data);
private:
    static const int32_t MAX_EPOLL_EVENTS;
private:
    int m_epollFd;
    std::atomic_bool m_running;
    struct epoll_event* m_epollEvts;

    struct EpollEvtDataHolder
    {
        uint32_t registeredEvts;
        bool isTriggeredEvent;
        std::shared_ptr<KMEpollable> epollablePtr;

        EpollEvtDataHolder() : registeredEvts(0), isTriggeredEvent(false) {}
    };
    std::set<EpollEvtDataHolder*> m_epollDataSet;
};

}; // namespace KM