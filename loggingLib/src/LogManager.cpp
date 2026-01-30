#include "LogManager.hpp"

void LogManager::route(const LogMessage &msg)
{
    for (const auto &sink : sinks)
    {
        auto sinkPtr = sink.get();        
        LogMessage copy = msg;            

        threadPool->enqueue([sinkPtr, copy]() mutable {
            sinkPtr->write(copy);
        });
    }
}

void LogManager::addSink(std::unique_ptr<ILogSink> sink)
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