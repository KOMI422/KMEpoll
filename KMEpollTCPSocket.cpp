#include "KMEpollTCPSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include "KMEpollUtils.h"

using namespace KM;

KMEpollTCPSocket::KMEpollTCPSocket(bool isListener, TCPSocketFamily sockType) 
    : MAX_BACKLOG(100), m_isListener(isListener), m_socketFamily(sockType)
{
    m_tcpFd = -1;
    m_tcpFd = socket(m_socketFamily, SOCK_STREAM | SOCK_NONBLOCK, 0);
    m_isNonBlock = true;
    m_isConnected = isListener;
    m_socketTimeOutTime = 0;
}

KMEpollTCPSocket::~KMEpollTCPSocket()
{
    if(m_tcpFd != -1)
    {
        close(m_tcpFd);
    }
}

void KMEpollTCPSocket::onEpollEvents(uint32_t evts, uint32_t timerMs)
{
    if(m_isListener)
    {
        handleListenerSocketEvents(evts);
    }
    else
    {
        if(evts == 0)   //Timer
        {
            checkSocketTimeOut(timerMs);
        }
        else
        {
            handleClientSocketEvents(evts);
        }
        
    }
    
}

bool KMEpollTCPSocket::isWaitingReadEvent() const
{
    return true;
}

bool KMEpollTCPSocket::isWaitingWriteEvent() const
{
    return (m_sendBuffer.size() > 0);
}

void KMEpollTCPSocket::setNonBlock(bool nonblock)
{
    int32_t flags = fcntl(m_tcpFd, F_GETFL);
    flags = nonblock ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    fcntl(m_tcpFd, F_SETFL, flags);
    m_isNonBlock = nonblock;
}

void KMEpollTCPSocket::listen(const std::string& ip, uint16_t port)
{
    if(!m_isListener)
        throw KMEpollException("KMEpollTCPSocket::listen not a listener TCPSocket");

    if(m_isNonBlock && !m_socketAcceptCb)
        throw KMEpollException("KMEpollTCPSocket::listen empty SocketAcceptCb for NonBlock listener TCPSocket");

    m_socketIpPort = KMEpollSocketIPPort::genSocketIPPort(
        ip, port, KMEpollSocketIPPort::TCP, (KMEpollSocketIPPort::SocketFamily) m_socketFamily);
    
    if(!m_socketIpPort)
        throw KMEpollException("KMEpollTCPSocket::listen invali ip or port err=" + std::string(strerror(errno)));

    if(bind(m_tcpFd, m_socketIpPort->getSockAddr(), m_socketIpPort->getSockLen()) != 0)
        throw KMEpollException("KMEpollTCPSocket::listen bind err=" + std::string(strerror(errno)));

    if(::listen(m_tcpFd, MAX_BACKLOG) != 0)
        throw KMEpollException("KMEpollTCPSocket::listen listen err=" + std::string(strerror(errno)));
}

void KMEpollTCPSocket::connect(const std::string& ip, uint16_t port)
{
    if(m_isListener)
        throw KMEpollException("KMEpollTCPSocket::connect not a client TCPSocket");

    if(m_tcpFd == -1)
        throw KMEpollException("KMEpollTCPSocket::connect invalid socket");

    m_socketIpPort = KMEpollSocketIPPort::genSocketIPPort(
        ip, port, KMEpollSocketIPPort::TCP, (KMEpollSocketIPPort::SocketFamily) m_socketFamily);

    if(!m_socketIpPort)
        throw KMEpollException("KMEpollTCPSocket::connect invali ip or port err=" + std::string(strerror(errno)));

    if(::connect(m_tcpFd, m_socketIpPort->getSockAddr(), m_socketIpPort->getSockLen()) != 0 && (errno != EINPROGRESS))
        throw KMEpollException("KMEpollTCPSocket::connect connect err=" + std::string(strerror(errno)));
}

std::shared_ptr<KMEpollTCPSocket> KMEpollTCPSocket::accept()
{
    if(!m_isListener)
        throw KMEpollException("KMEpollTCPSocket::accept not a listener TCPSocket");

    struct sockaddr sockAddr;
    memset(&sockAddr, 0, sizeof(struct sockaddr));
    socklen_t sockLen = 0;

    int newSockFd = ::accept(m_tcpFd, &sockAddr, &sockLen);
    if(newSockFd == -1 && (errno != EAGAIN && errno != EWOULDBLOCK))
    {
        throw KMEpollException("KMEpollTCPSocket::accept accept socket err=" + std::string(strerror(errno)));
    }

    std::shared_ptr<KMEpollTCPSocket> pAcceptedSock;
    
    if(newSockFd > 0)
    {
        pAcceptedSock = std::make_shared<KMEpollTCPSocket>(false, m_socketFamily);
        pAcceptedSock->m_tcpFd = newSockFd;
        pAcceptedSock->m_socketIpPort = KMEpollSocketIPPort::genSocketIPPort(&sockAddr, sockLen);
        pAcceptedSock->m_isConnected = true;
    }

    return pAcceptedSock;
}

int32_t KMEpollTCPSocket::recv(void* recvBuffer, int32_t recvSize)
{
    if(m_isListener)
        throw KMEpollException("KMEpollTCPSocket::recv not a client TCPSocket");

    int nRead = ::recv(m_tcpFd, recvBuffer, recvSize, 0);

    if(nRead < 0)
    {
        if(errno != EAGAIN && errno != EINTR)
        {
            throw KMEpollException("KMEpollTCPSocket::recv recv data exception err:" + std::string(strerror(errno)));
        }

        return -1;
    }

    return nRead;
}

int32_t KMEpollTCPSocket::send(const void* sendBuffer, int32_t sendSize)
{
    if(m_isListener)
        throw KMEpollException("KMEpollTCPSocket::send not a client TCPSocket");

    int nSent = ::send(m_tcpFd, sendBuffer, sendSize, 0);

    if(nSent < 0)
    {
        if(errno != EAGAIN && errno != EINTR)
        {
            throw KMEpollException("KMEpollTCPSocket::recv send data exception err:" + std::string(strerror(errno)));
        }

        return -1;
    }

    return nSent;
}

void KMEpollTCPSocket::closeSocket()
{
    if(m_tcpFd == -1)
        throw KMEpollException("KMEpollTCPSocket::closeSocket invalid socket");

    ::close(m_tcpFd);
    m_tcpFd = -1;
    m_isConnected = false;
    setSocketTimeOut(1000);
}

void KMEpollTCPSocket::handleListenerSocketEvents(uint32_t evts)
{
    if(isReadable(evts) || isWritable(evts))
    {
        std::shared_ptr<KMEpollTCPSocket> pNewSocket = this->accept();
        if(pNewSocket)
        {
            m_socketAcceptCb(pNewSocket);
        }
    }
}

void KMEpollTCPSocket::handleClientSocketEvents(uint32_t evts)
{
    bool shouldCloseSocket = false;
    TCPSocketClosedReason reason;
    
    try
    {
        if (!shouldCloseSocket && isReadable(evts))
        {
            char buf[512 * 1024] = {0};
            while (true)
            {
                int nRead = this->recv(buf, sizeof(buf));
                if (nRead == -1) //当前没数据
                    break;

                int32_t readCbRet = 0;
                if (nRead > 0)
                {
                    m_recvBuffer.append(buf, nRead);
                    readCbRet = m_socketDataCb(
                        shared_from_this(), m_recvBuffer.data(), m_recvBuffer.size());
                }

                if (nRead == 0 || readCbRet < 0)
                {
                    shouldCloseSocket = true;
                    reason = (nRead == 0) ? PASSIVE_CLOSED : INITIATIVE_CLOSED;
                    break;
                }

                m_recvBuffer = m_recvBuffer.substr(readCbRet);
            }
        }

        if (!shouldCloseSocket && isWritable(evts))
        {
            int nSent = 0;
            if (!m_sendBuffer.empty())
            {
                nSent = this->send(m_sendBuffer.data(), m_sendBuffer.size());
                if (nSent > 0)
                {
                    m_sendBuffer = m_sendBuffer.substr(nSent);
                }
            }
        }
    }
    catch(KMEpollException& ex)
    {
        shouldCloseSocket = true;
        reason = ERRONEOUS_CLOSED;
    }

    if(shouldCloseSocket && m_socketClosedCb)
    {
        m_socketClosedCb(shared_from_this(), reason);
        closeSocket();
    }
}

bool KMEpollTCPSocket::isSocketTimeOut() const
{
    return (!m_isConnected && m_socketTimeOutTime > 0 && KMEpollUtils::getNowMs() >= m_socketTimeOutTime);
}

void KMEpollTCPSocket::setSocketTimeOut(uint32_t intervalMs)
{
    m_socketTimeOutTime = (intervalMs == 0) ? 0 : (intervalMs + KMEpollUtils::getNowMs());
}

void KMEpollTCPSocket::checkSocketTimeOut(uint32_t nowMs)
{
    if(isSocketTimeOut())
        setDestroying(true);
}
