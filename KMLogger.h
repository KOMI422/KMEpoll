#pragma once
#include <iostream>

#define KMLOG() KM::KMLogger::getInstance()->log()
#define FUNC_LOG() KM::KMLogger::getInstance()->log() << __FUNCTION__
#define FUNC_DEBUG() KM::KMLogger::getInstance()->debug() << __FUNCTION__

namespace KM
{

class KMLogger
{
public:
    static KMLogger* getInstance();
    std::ostream& log();
    std::ostream& debug();
private:
    static KMLogger* m_pInstance;
};

};