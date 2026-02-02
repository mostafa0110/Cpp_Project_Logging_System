#include "LogManager.hpp"

void LogManager::route(const LogMessage &msg)
{
    for (const auto &sink : sinks)
    {
        // Capture shared_ptr by value to extend sink lifetime
        auto sinkCopy = sink;
        LogMessage msgCopy = msg;

        threadPool->enqueue([sinkCopy, msgCopy]() mutable {
            sinkCopy->write(msgCopy);
        });
    }
}

void LogManager::addSink(std::shared_ptr<ILogSink> sink)
{
    sinks.push_back(std::move(sink));
}

void LogManager::log(const LogMessage &msg)
{
    if (!buffer.tryPush(msg))
    {
        flush();
        (void)buffer.tryPush(msg);
    }
}

void LogManager::flush()
{
    while (auto msg = buffer.tryPop())
    {
        route(msg.value());
    }
}