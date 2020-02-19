#pragma once

#include "KMEpoll.h"
#include "KMEpollSocketIPPort.h"
#include <memory>
#include <functional>

namespace KM
{

class KMEpollTCPSocket : public KMEpollable, public std::enable_shared_from_this<KMEpollTCPSocket>
{
public:
    enum TCPSocketFamily
    {
        IPv4_Socket = KMEpollSocketIPPort::IPv4,
        IPv6_Socket = KMEpollSocketIPPort::IPv6
    };
    typedef std::function<void(std::shared_ptr<KMEpollTCPSocket> pSocket)> SocketAcceptCb;
    typedef std::function<int32_t(std::shared_ptr<KMEpollTCPSocket> pSocket, const char* data, uint32_t len)> SocketDataCb;

    enum TCPSocketClosedReason
    {
        INITIATIVE_CLOSED = 0,
        PASSIVE_CLOSED = 1,
        ERRONEOUS_CLOSED = 2
    };
    typedef std::function<void(std::shared_ptr<KMEpollTCPSocket> pSocket, TCPSocketClosedReason reason)> SocketClosedCb;
public:
    KMEpollTCPSocket(bool isListener, TCPSocketFamily sockType = IPv4_Socket);
    virtual ~KMEpollTCPSocket();
    virtual void onEpollEvents(uint32_t evts, uint32_t timerMs);
    virtual int32_t getEpollableFd() const { return m_tcpFd; }
    virtual bool isWaitingReadEvent() const;
    virtual bool isWaitingWriteEvent() const;

    void setNonBlock(bool nonblock);
    void listen(const std::string& ip, uint16_t port);
    void connect(const std::string& ip, uint16_t port);
    std::shared_ptr<KMEpollTCPSocket> accept();
    int32_t recv(void* recvBuffer, int32_t recvSize);
    int32_t send(const void* sendBuffer, int32_t sendSize);
    void closeSocket();

    TCPSocketFamily getSocketFamily() const { return m_socketFamily; }
    const std::shared_ptr<KMEpollSocketIPPort>& getSocketIPPort() const { return m_socketIpPort; }

    void setSocketAcceptCallback(SocketAcceptCb cb) { m_socketAcceptCb = cb; }
    void setSocketDataCallback(SocketDataCb cb) { m_socketDataCb = cb; }
    void setSocketClosedCallback(SocketClosedCb cb) { m_socketClosedCb = cb; }
    bool isSocketTimeOut() const;
private:
    void handleListenerSocketEvents(uint32_t evts);
    void handleClientSocketEvents(uint32_t evts);
    void setSocketTimeOut(uint32_t intervalMs);
    void checkSocketTimeOut(uint32_t nowMs);
private:
    const uint32_t MAX_BACKLOG;
    const bool m_isListener;
    const TCPSocketFamily m_socketFamily;
    bool m_isNonBlock;
    int32_t m_tcpFd;
    std::shared_ptr<KMEpollSocketIPPort> m_socketIpPort;
    bool m_isConnected;
    uint64_t m_socketTimeOutTime;

    std::string m_recvBuffer;
    std::string m_sendBuffer;

    SocketAcceptCb m_socketAcceptCb;
    SocketDataCb m_socketDataCb;
    SocketClosedCb m_socketClosedCb;
};

};