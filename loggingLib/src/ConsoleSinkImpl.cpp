#include "ConsoleSinkImpl.hpp"
#include <iostream>


void ConsoleSinkImpl::write(const LogMessage & msg){
    std::cout << msg << std::endl;
}