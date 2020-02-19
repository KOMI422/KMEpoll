#pragma once

#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace KM
{

class KMEpollSocketIPPort
{
public:
    enum SocketType
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum SocketFamily
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6
    };

    static std::shared_ptr<KMEpollSocketIPPort> genSocketIPPort(
        const std::string& ip, uint16_t port, SocketType type = TCP, SocketFamily family = IPv4);
    static std::shared_ptr<KMEpollSocketIPPort> genSocketIPPort(
        const struct sockaddr* pAddr, socklen_t socklen);
public:
    virtual ~KMEpollSocketIPPort();

    const struct sockaddr* getSockAddr() const { return m_pSocketAddr; }
    socklen_t getSockLen() const { return m_socketLen; }
    std::string getIpString() const { return m_ipString; }
    uint16_t getPort() const { return m_port; }
protected:
    KMEpollSocketIPPort();
private:
    socklen_t m_socketLen;
    struct sockaddr* m_pSocketAddr;
    std::string m_ipString;
    uint16_t m_port;
};

};