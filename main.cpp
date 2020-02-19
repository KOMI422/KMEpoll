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

    void onAcceptSocket(std::shared_ptr<KMEpollTCPSocket> pSocket)
    {
        pSocket->setSocketDataCallback(
            std::bind(&SimpleServer::onReadSocket, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );
        kmEpoll.registerEpollable(pSocket);
        std::string msg = "hello world";
        pSocket->send(msg.data(), msg.size());
    }
};

int main(int argc, char* argv[])
{
    SimpleServer svr;

    std::shared_ptr<KMEpollTCPSocket> pListener = std::make_shared<KMEpollTCPSocket>(true);
    pListener->setSocketAcceptCallback(std::bind(&SimpleServer::onAcceptSocket, &svr, std::placeholders::_1));
    pListener->listen("localhost", 12345);
    kmEpoll.registerEpollable(pListener);

    kmEpoll.runEpoll();

    return 0;
}