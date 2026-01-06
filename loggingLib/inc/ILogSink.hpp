#pragma once

#include "LogMessage.hpp"

class ILogSink {
    public:
        virtual void write(const LogMessage & msg) = 0;
        virtual ~ILogSink() = default;
};