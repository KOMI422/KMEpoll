#include "KMEpollSocketIPPort.h"
#include <cstring>
#include <stdlib.h>

using namespace KM;

std::shared_ptr<KMEpollSocketIPPort> KMEpollSocketIPPort::genSocketIPPort(
    const std::string& ip, uint16_t port, SocketType type, SocketFamily family)
{
    std::shared_ptr<KMEpollSocketIPPort> pSocketIp;

    struct addrinfo hint;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_family = family;
    hint.ai_socktype = type;
    hint.ai_protocol = 0;
    hint.ai_flags |= AI_NUMERICSERV;
    if(ip.empty())
        hint.ai_flags |= AI_PASSIVE;

    char strPort[10] = { 0 };
    snprintf(strPort, sizeof(strPort), "%u", port);

    struct addrinfo* pResultInfo = NULL;
    if(getaddrinfo(ip.empty() ? NULL : ip.c_str(), strPort, &hint, &pResultInfo) == 0)
    {
        pSocketIp = std::shared_ptr<KMEpollSocketIPPort>(new KMEpollSocketIPPort());
        pSocketIp->m_port = port;
        pSocketIp->m_socketLen = pResultInfo->ai_addrlen;
        pSocketIp->m_pSocketAddr = (struct sockaddr*)malloc(pSocketIp->m_socketLen);
        memcpy(pSocketIp->m_pSocketAddr, pResultInfo->ai_addr, pSocketIp->m_socketLen);

        freeaddrinfo(pResultInfo);
    }

    return pSocketIp;
}

std::shared_ptr<KMEpollSocketIPPort> KMEpollSocketIPPort::genSocketIPPort(
        const struct sockaddr* pAddr, socklen_t socklen)
{
    std::shared_ptr<KMEpollSocketIPPort> pSocketIp = std::shared_ptr<KMEpollSocketIPPort>(new KMEpollSocketIPPort());
    pSocketIp->m_socketLen = socklen;
    pSocketIp->m_pSocketAddr = (struct sockaddr*)malloc(pSocketIp->m_socketLen);
    memcpy(pSocketIp->m_pSocketAddr, pAddr, pSocketIp->m_socketLen);

    return pSocketIp;
}

KMEpollSocketIPPort::KMEpollSocketIPPort()
{
    m_port = 0;
    m_socketLen = 0;
    m_pSocketAddr = NULL;
}

KMEpollSocketIPPort::~KMEpollSocketIPPort()
{
    if(m_pSocketAddr != NULL)
        free(m_pSocketAddr);
}