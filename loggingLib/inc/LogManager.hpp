#pragma once

#include "ILogSink.hpp"
#include <memory>
#include <vector>
#include "RingBuffer.hpp"
#include "LogMessage.hpp"

class LogManager
{
private:
    static constexpr std::size_t DEFAULT_BUFFER_CAPACITY = 100;

    std::vector<std::unique_ptr<ILogSink>> sinks;
    RingBuffer<LogMessage> buffer;
    void route(const LogMessage &msg);

public:
    explicit LogManager(std::size_t bufferCapacity = DEFAULT_BUFFER_CAPACITY)
        : buffer(bufferCapacity)
    {
    }

    // Non-copyable, non-movable
    LogManager(const LogManager &other) = delete;
    LogManager(LogManager &&other) = delete;
    LogManager &operator=(const LogManager &other) = delete;
    LogManager &operator=(LogManager &&other) = delete;

    void addSink(std::unique_ptr<ILogSink> sink);
    void log(const LogMessage &msg);
    void flush();
};