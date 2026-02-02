#pragma once

#include "interfaces/ILogSink.hpp"
#include <memory>
#include <vector>
#include "concurrency/RingBuffer.hpp"
#include "LogMessage.hpp"
#include "concurrency/ThreadPool.hpp"

class LogManager
{
private:
    static constexpr std::size_t DEFAULT_BUFFER_CAPACITY = 100;
    static constexpr std::size_t DEFAULT_THREAD_COUNT = 4;

    std::vector<std::shared_ptr<ILogSink>> sinks;
    RingBuffer<LogMessage> buffer;
    std::unique_ptr<ThreadPool> threadPool;

    void route(const LogMessage &msg);

public:
    explicit LogManager(
        std::size_t bufferCapacity = DEFAULT_BUFFER_CAPACITY, 
        std::size_t numThreads = DEFAULT_THREAD_COUNT)
        : buffer(bufferCapacity),
          threadPool(std::make_unique<ThreadPool>(numThreads))
    {
    }

    // Non-copyable, non-movable
    LogManager(const LogManager &other) = delete;
    LogManager(LogManager &&other) = delete;
    LogManager &operator=(const LogManager &other) = delete;
    LogManager &operator=(LogManager &&other) = delete;

    void addSink(std::shared_ptr<ILogSink> sink);
    void log(const LogMessage &msg);
    void flush();
};