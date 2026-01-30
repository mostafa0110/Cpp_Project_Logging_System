#pragma once

#include "ILogSink.hpp"
#include "LogTypes.hpp"
#include <memory>
#include <string>
#include <expected>


enum class SinkCreationError {
    MISSING_FILEPATH,
    MISSING_SOCKET_ADDRESS,
    UNKNOWN_SINK_TYPE,
};


class LogSinkFactory
{
public:
    LogSinkFactory() = delete;
    static std::expected<std::shared_ptr<ILogSink>, SinkCreationError> create(LogSinkType type, const std::string &config = "");
};