#include "ConsoleSinkImpl.hpp"
#include <iostream>

std::mutex ConsoleSinkImpl::coutMutex;

void ConsoleSinkImpl::write(const LogMessage &msg)
{
    std::lock_guard<std::mutex> lock(coutMutex);
    std::cout << msg << '\n';
}