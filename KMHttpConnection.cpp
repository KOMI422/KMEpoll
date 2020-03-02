#include "KMHttpConnection.h"
#include <regex>

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
    m_connSocket = std::make_shared<KMEpollTCPSocket>(false);
}

bool KMHttpConnection::connectUrl(const std::string& url)
{
    if(!m_httpUrl.parseUrl(url))
        return false;

    
    return true;
}