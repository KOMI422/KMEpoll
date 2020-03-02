#pragma once

#include <string>
#include "KMEpollTCPSocket.h"
#include "KMEpoll.h"
#include <map>

namespace KM
{

class KMHttpUrl
{
public:
    static const std::string HTTP_URL_REG_PATTERN;
public:
    KMHttpUrl();
    
    bool parseUrl(const std::string& url);
    std::string genUrl() const;

    const std::string& getSchema() const { return m_schema; }

    const std::string& getHost() const { return m_host; }
    void setHost(const std::string& host) { m_host = host; }

    uint16_t getPort() const { return m_port; }
    void setPort(uint16_t port) { m_port = port; }

    const std::string& getRelativePath() const { return m_relativePath; }
    void setRelativePath(const std::string& path) { m_relativePath = path; }

    const std::string& getQueryString() const { return m_queryString; }
    std::string getQueryParams(const std::string& paramKey, const std::string& defaultValue) const;
    void setQueryParams(const std::string& paramKey, const std::string& paramValue);
private:
    std::string m_urlString;

    std::string m_schema;
    std::string m_host;
    uint16_t m_port;
    std::string m_relativePath;
    std::string m_queryString;
    std::map<std::string, std::string> m_queryParams;
};

class KMHttpConnection
{
public:
    KMHttpConnection();

    bool connectUrl(const std::string& url);
private:
    KMEpollTCPSocket_Ptr m_connSocket;
    KMHttpUrl m_httpUrl;
};

};