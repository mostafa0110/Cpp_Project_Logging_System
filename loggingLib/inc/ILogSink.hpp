#pragma once

#include "LogMessage.hpp"

class ILogSink
{
public:
    virtual ~ILogSink() noexcept = default;
    virtual void write(const LogMessage &msg) = 0;

protected:
    ILogSink() = default;
    ILogSink(const ILogSink &) = default;
    ILogSink &operator=(const ILogSink &) = default;
    ILogSink(ILogSink &&) = default;
    ILogSink &operator=(ILogSink &&) = default;
};