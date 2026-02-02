#pragma once

#include <string>
#include <ostream>
#include "LogTypes.hpp"

class LogMessage
{
private:
    TelemetrySrc source;
    SeverityLvl severity;
    std::string timeStamp;
    std::string payload;

public:
    LogMessage() = delete;
    LogMessage(TelemetrySrc source,
               SeverityLvl severity,
               std::string timeStamp,
               std::string payload);

    LogMessage(const LogMessage &) = default;
    LogMessage(LogMessage &&) = default;
    LogMessage &operator=(const LogMessage &) = default;
    LogMessage &operator=(LogMessage &&) = default;
    ~LogMessage() = default;

    friend std::ostream &operator<<(std::ostream &os, const LogMessage &msg);
};