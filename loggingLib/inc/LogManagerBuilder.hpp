#pragma once

#include "LogManager.hpp"
#include "LogSinkFactory.hpp"
#include "LogTypes.hpp"
#include <vector>
#include <string>
#include <memory>
#include <expected>

enum class BuilderError
{
    NO_SINKS_CONFIGURED,
    INVALID_BUFFER_SIZE,
    INVALID_THREADPOOL_SIZE,
    EMPTY_FILEPATH,
    NULL_SINK,
    SINK_CREATION_FAILED
};

class LogManagerBuilder
{
private:
    std::vector<std::shared_ptr<ILogSink>> sinks;
    std::size_t bufferSize = 100;
    std::size_t threadPoolSize = 4;
    std::vector<BuilderError> errors;

public:
    LogManagerBuilder() = default;

    LogManagerBuilder &withConsoleSink();
    LogManagerBuilder &withFileSink(const std::string &filepath);
    LogManagerBuilder &withSink(std::shared_ptr<ILogSink> sink);
    LogManagerBuilder &withSink(LogSinkType type, const std::string &config = "");
    LogManagerBuilder &withBufferSize(std::size_t size);
    LogManagerBuilder &withthreadPoolSize(std::size_t size);

    [[nodiscard]] std::unique_ptr<LogManager> build();
    [[nodiscard]] std::expected<std::unique_ptr<LogManager>, BuilderError> tryBuild();

    LogManagerBuilder &reset();
};