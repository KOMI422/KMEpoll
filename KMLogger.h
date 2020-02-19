#pragma once
#include <iostream>

namespace KM
{

class KMLogger
{
public:
    static KMLogger* getInstance();
    std::ostream& getLogFunc();

private:
    static KMLogger* m_pInstance;
};

};