#pragma once

#include <string>
#include "KMEpollTCPSocket.h"
#include "KMEpoll.h"
#include <map>

extern "C"
{
    #include "http_parser.h"
}

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
    typedef std::function<void(const char* data, int32_t len)> HttpDataCallback;
    typedef std::function<void()> HttpConnectionClosedCallback;
public:
    KMHttpConnection();
    virtual ~KMHttpConnection();

    bool connectUrl(const std::string& url, bool nonBlock = true);
    KMEpollTCPSocket_Ptr getSocket() { return m_connSocket; }

    int32_t readData(void* buf, int32_t len);
    int32_t writeData(const void* buf, int32_t len);
private:
    static int onHttpParserData(http_parser* pParser, const char * data, size_t len);
private:
    void initHttpParser();
private:
    KMEpollTCPSocket_Ptr m_connSocket;
    KMHttpUrl m_httpUrl;

    http_parser* m_pHttpParser;
    http_parser_settings m_httpParserSetting;
};

};