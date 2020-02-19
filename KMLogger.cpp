#include "KMLogger.h"

using namespace KM;

KMLogger* KMLogger::m_pInstance = NULL;

KMLogger* KMLogger::getInstance()
{
    if(m_pInstance == NULL)
        m_pInstance = new KMLogger();

    return m_pInstance;
}

std::ostream& KMLogger::getLogFunc()
{
    return std::cout;
}