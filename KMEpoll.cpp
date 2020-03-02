#include "KMEpoll.h"
#include <unistd.h>
#include <cstring>
#include "KMLogger.h"
#include "KMEpollUtils.h"

using namespace KM;

bool KMEpollable::isReadable(uint32_t evts)
{
    return (evts & EPOLLIN) > 0;
}

bool KMEpollable::isWritable(uint32_t evts)
{
    return (evts & EPOLLOUT) > 0;
}

bool KMEpollable::isError(uint32_t evts, int32_t& errNo)
{
    errNo = 0;

    if((evts & EPOLLERR) > 0 && (errno != EINTR && errno != EAGAIN))
    {
        errNo = errno;
        return true;
    }

    return false;
}

const int32_t KMEpoll::MAX_EPOLL_EVENTS = 100;

KMEpoll::KMEpoll()
{
    m_epollFd = -1;
    m_epollFd = epoll_create(MAX_EPOLL_EVENTS);

    m_epollEvts = new epoll_event[MAX_EPOLL_EVENTS];
}

KMEpoll::~KMEpoll()
{
    if(m_epollFd != -1)
    {
        ::close(m_epollFd);
    }

    delete[] m_epollEvts;

    for(EpollEvtDataHolder* pHolder : m_epollDataSet)
    {
        delete pHolder;
    }
    m_epollDataSet.clear();
}

void KMEpoll::runEpoll()
{
    m_running = true;

    while(m_running)
    {
        int evtRet = epoll_wait(m_epollFd, m_epollEvts, MAX_EPOLL_EVENTS, 500);
        if(evtRet == -1)
        {
            if(evtRet != EAGAIN && evtRet != EINTR)
            {
                KMLogger::getInstance()->log() << "KMEpoll::runEpoll wait err=" << evtRet 
                    << " errMsg=" << strerror(evtRet) << std::endl;
            }
            continue;
        }

        for(int i = 0; i < evtRet; i++)
        {
            struct epoll_event& curEvt = m_epollEvts[i];
            EpollEvtDataHolder* pDataHolder = (EpollEvtDataHolder*)curEvt.data.ptr;

            try
            {
                pDataHolder->isTriggeredEvent = true;
                pDataHolder->epollablePtr->onEpollEvents(curEvt.events);
            }
            catch(std::exception& ex)
            {
                KMLogger::getInstance()->log() << "KMEpoll::runEpoll onEpollEvents exception=" 
                    << ex.what() << " epollableFd=" << pDataHolder->epollablePtr->getEpollableFd() << std::endl;
                pDataHolder->epollablePtr->setCanDestroy(true);
            }
        }

        for(std::set<KMEpoll::EpollEvtDataHolder*>::iterator itData = m_epollDataSet.begin();
            itData != m_epollDataSet.end();
            )
        {
            EpollEvtDataHolder* pHolder = (*itData);
            if(pHolder->epollablePtr->checkTimeOut(KMEpollUtils::getNowMs()))
            {
                KMLogger::getInstance()->log() << "KMEpoll socket timeout" << std::endl;
                pHolder->epollablePtr->setCanDestroy(true);
            }
            
            if(pHolder->epollablePtr->canDestroy())
            {
                setEpollCtrl(pHolder->epollablePtr, EPOLL_CTL_DEL, 0, NULL);

                itData = m_epollDataSet.erase(itData);
                delete pHolder;
            }
            else if(pHolder->isTriggeredEvent)
            {
                pHolder->isTriggeredEvent = false;

                uint32_t waitEvts = 0;
                waitEvts |= (pHolder->epollablePtr->isWaitingReadEvent() ? EPOLLIN : 0);
                waitEvts |= (pHolder->epollablePtr->isWaitingWriteEvent() ? EPOLLOUT : 0);
                waitEvts |= (pHolder->epollablePtr->isEdgeTrigger() ? EPOLLET : 0);

                if(pHolder->registeredEvts != waitEvts)
                {
                    pHolder->registeredEvts = waitEvts;
                    setEpollCtrl(pHolder->epollablePtr, EPOLL_CTL_MOD, pHolder->registeredEvts, pHolder);
                }

                itData++;
            }
            else
            {
                itData++;
            }
            
        }
    }
}

void KMEpoll::registerEpollable(std::shared_ptr<KMEpollable> pEpollablePtr)
{
    EpollEvtDataHolder* pHolder = new EpollEvtDataHolder();
    pHolder->epollablePtr = pEpollablePtr;
    pHolder->registeredEvts = (EPOLLIN | EPOLLOUT);
    m_epollDataSet.insert(pHolder);

    setEpollCtrl(pEpollablePtr, EPOLL_CTL_ADD, pHolder->registeredEvts, pHolder);
}

void KMEpoll::setEpollCtrl(std::shared_ptr<KMEpollable> pEpollable, int32_t op, uint32_t evts, void* data)
{
    if(op == EPOLL_CTL_DEL)
    {
        epoll_ctl(m_epollFd, op, pEpollable->getEpollableFd(), NULL);
    }
    else
    {
        struct epoll_event evt;
        evt.events = evts;
        evt.data.ptr = data;
        epoll_ctl(m_epollFd, op, pEpollable->getEpollableFd(), &evt);
    }
}
