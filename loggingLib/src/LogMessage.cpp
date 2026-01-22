#include "LogMessage.hpp"
#include <magic_enum/magic_enum.hpp>

LogMessage::LogMessage(TelemetrySrc source,
                       SeverityLvl severity,
                       std::string timeStamp,
                       std::string payload)
    : source(source),
      severity(severity),
      timeStamp(std::move(timeStamp)),
      payload(std::move(payload)) {}

std::ostream &operator<<(std::ostream &os, const LogMessage &msg)
{
    os << "[" << magic_enum::enum_name(msg.source) << "] "
       << "[" << magic_enum::enum_name(msg.severity) << "] "
       << "[" << msg.timeStamp << "] "
       << msg.payload;

    return os;
}