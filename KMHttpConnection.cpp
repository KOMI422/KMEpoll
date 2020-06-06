#include "KMHttpConnection.h"
#include <regex>
#include "KMEpollSocketIPPort.h"
#include "KMLogger.h"

using namespace KM;

const std::string KMHttpUrl::HTTP_URL_REG_PATTERN = "(http|https)://((.+):(.+)@)?([\\w\\.]+)(:(\\d+))?(/|/[\\w\\.%]+)?(\\?|\\?([\\w=&%]+))?(#\\w+)?";

KMHttpUrl::KMHttpUrl()
{
    m_port = 0;
}

bool KMHttpUrl::parseUrl(const std::string& url)
{
    std::regex reg(HTTP_URL_REG_PATTERN);
    std::smatch rMatch;

    if(!std::regex_match(url, rMatch, reg))
        return false;

    m_urlString = url;
    m_schema = rMatch[1].str();
    m_host = rMatch[5].str();

    std::string portStr = rMatch[7].str();
    m_port = portStr.empty() ? 80 : (uint16_t)strtoul(portStr.c_str(), NULL, 0);
    m_relativePath = rMatch[8].str();
    m_queryString = rMatch[10].str();

    return true;
}

KMHttpConnection::KMHttpConnection()
{
    // m_connSocket = std::make_shared<KMEpollTCPSocket>(false);
    m_pHttpParser = (http_parser*)malloc(sizeof(http_parser));
    m_pHttpParser->data = this;

    initHttpParser();
}

KMHttpConnection::~KMHttpConnection()
{
    if(m_pHttpParser != NULL)
    {
        free(m_pHttpParser);
        m_pHttpParser = NULL;
    }
}

bool KMHttpConnection::connectUrl(const std::string& url, bool nonBlock)
{
    if(!m_httpUrl.parseUrl(url))
        return false;

    m_connSocket = std::make_shared<KMEpollTCPSocket>(false);
    if(nonBlock)
    {
        m_connSocket->setNonBlock(true);
        m_connSocket->setSocketDataCallback(
            [&](std::shared_ptr<KMEpollTCPSocket> pSocket, const char *data, uint32_t len) -> int32_t {
                int parsed = http_parser_execute(m_pHttpParser, &m_httpParserSetting, data, len);
                if(parsed != len)
                {
                    //TODO error
                    return -1;
                }
                else
                {
                    return len;
                }
            });
        m_connSocket->setSocketConnectedCallback(
            [this](std::shared_ptr<KMEpollTCPSocket> pSocket) -> void {
                KMLogger::getInstance()->log() << "connected" << std::endl;
                std::string req = "GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n";
                pSocket->send(req.data(), req.size());
            });
        m_connSocket->setSocketClosedCallback(
            [this](std::shared_ptr<KMEpollTCPSocket> pSocket,
                   KMEpollTCPSocket::TCPSocketClosedReason reason) -> void {
                KMLogger::getInstance()->log() << "closed" << std::endl;
            });
    }

    m_connSocket->connect(m_httpUrl.getHost(), m_httpUrl.getPort());
    
    return true;
}

void KMHttpConnection::closeConnection()
{
    if(m_connSocket)
    {
        m_connSocket->closeSocket();
    }
}

int32_t KMHttpConnection::readData(void* buf, int32_t len)
{
    return m_connSocket->recv(buf, len);
}

int32_t KMHttpConnection::writeData(const void* buf, int32_t len)
{
    return m_connSocket->send(buf, len);
}

void KMHttpConnection::initHttpParser()
{
    http_parser_init(m_pHttpParser, HTTP_RESPONSE);

    http_parser_settings_init(&m_httpParserSetting);
    m_httpParserSetting.on_body = &KMHttpConnection::onHttpParserData;
}

int KMHttpConnection::onHttpParserData(http_parser* pParser, const char * data, size_t len)
{
    std::string rsp(data, data + len);
    FUNC_DEBUG() << rsp << std::endl;
    return 0;
}