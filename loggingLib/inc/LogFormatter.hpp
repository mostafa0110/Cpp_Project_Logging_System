#pragma once

#include "LogTypes.hpp"
#include <optional>
#include "LogMessage.hpp"
#include "LogPolicies.hpp"
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <charconv>
#include <magic_enum/magic_enum.hpp>

template <typename Policy>
class LogFormatter
{
public:
    [[nodiscard]] std::optional<LogMessage> formatDataToLogMsg(const std::string &raw);

private:
    [[nodiscard]] std::string msgDescription(float val, SeverityLvl severity);
    [[nodiscard]] std::string currentTimeStamp();
};

template <typename Policy>
std::optional<LogMessage> LogFormatter<Policy>::formatDataToLogMsg(const std::string &raw)
{
    if (raw.empty())
    {
        return std::nullopt;
    }

    float val = std::stof(raw);
    SeverityLvl severity = Policy::inferSeverity(val);

    return LogMessage(
        Policy::context,
        severity,
        currentTimeStamp(),
        msgDescription(val, severity));
}

template <typename Policy>
std::string LogFormatter<Policy>::currentTimeStamp()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

template <typename Policy>
std::string LogFormatter<Policy>::msgDescription(float val, SeverityLvl severity)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);

    // Source and current value
    oss << magic_enum::enum_name(Policy::context) << ": "
        << val << " " << Policy::unit;

    // Status based on severity
    switch (severity)
    {
    case SeverityLvl::CRITICAL:
        oss << " | CRITICAL! Exceeded threshold (" << Policy::CRITICAL << Policy::unit << ")";
        break;
    case SeverityLvl::WARNING:
        oss << " | Warning: Above normal (" << Policy::WARNING << Policy::unit
            << "), approaching critical (" << Policy::CRITICAL << Policy::unit << ")";
        break;
    case SeverityLvl::INFO:
        oss << " | Status: Normal (threshold: " << Policy::WARNING << Policy::unit << ")";
        break;
    }

    return oss.str();
}