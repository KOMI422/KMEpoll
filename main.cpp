#include "KMEpoll.h"
#include "KMEpollTCPSocket.h"
#include "KMEpollTimer.h"
#include <iostream>
#include <functional>

using namespace KM;

KMEpoll kmEpoll;

class SimpleServer
{
public:
    int32_t onReadSocket(std::shared_ptr<KMEpollTCPSocket> pSocket, const char* data, uint32_t len)
    {
        std::cout << data << std::endl;
        return len;
    }

    void onSocketClosed(std::shared_ptr<KMEpollTCPSocket> pSocket, KMEpollTCPSocket::TCPSocketClosedReason reason)
    {
        std::cout << " closed reason=" << reason << std::endl;
    }

    void onAcceptSocket(std::shared_ptr<KMEpollTCPSocket> pSocket)
    {
        pSocket->setSocketDataCallback(
            std::bind(&SimpleServer::onReadSocket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );
        pSocket->setSocketClosedCallback(
            std::bind(&SimpleServer::onSocketClosed, this, std::placeholders::_1, std::placeholders::_2)
        );
        // pSocket->setTimeOut(10 * 1000);
        kmEpoll.registerEpollable(pSocket);
        std::string msg = "hello world";
        pSocket->send(msg.data(), msg.size());
    }
};

#include "KMHttpConnection.h"
int main(int argc, char* argv[])
{
    // std::string url = "https://www.baidu.com:80/index.jsp?param=test#tag";
    // KMHttpUrl httpUrl;
    // httpUrl.parseUrl(url);
    // std::cout << "schema=" << httpUrl.getSchema() << " host=" << httpUrl.getHost() << " port=" << httpUrl.getPort()
    //     << " path=" << httpUrl.getRelativePath() << " query=" << httpUrl.getQueryString() << std::endl;

    // return 0;

    KMEpoll epoller;

    std::shared_ptr<KMHttpConnection> pConn = std::make_shared<KMHttpConnection>();
    pConn->connectUrl("http://www.baidu.com");

    epoller.registerEpollable(pConn->getSocket());
    epoller.runEpoll();

    return 0;

    SimpleServer svr;

    std::shared_ptr<KMEpollTCPSocket> pListener = std::make_shared<KMEpollTCPSocket>(true);
    pListener->setSocketAcceptCallback(std::bind(&SimpleServer::onAcceptSocket, &svr, std::placeholders::_1));
    pListener->listen("localhost", 12345);
    kmEpoll.registerEpollable(pListener);

    kmEpoll.runEpoll();

    return 0;
}