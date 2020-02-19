#include "KMEpollUtils.h"
#include <sys/time.h>
#include <iostream>

using namespace KM;

uint64_t KMEpollUtils::getNowMs()
{
    struct timeval nowTs;
    gettimeofday(&nowTs, NULL);
    return (nowTs.tv_sec * 1000 + nowTs.tv_usec / 1000);
}