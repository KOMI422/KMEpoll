#pragma once
#include <iostream>

#define KMLOG KM::KMLogger::getInstance()->log()

namespace KM
{

class KMLogger
{
public:
    static KMLogger* getInstance();
    std::ostream& log();

private:
    static KMLogger* m_pInstance;
};

};