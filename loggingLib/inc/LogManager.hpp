#pragma once 

#include "ILogSink.hpp"
#include <memory>
#include <vector>

class LogManager {
    private:
        std::vector <std::unique_ptr<ILogSink>>  sinks;
        std::vector <LogMessage> buffer;
        //consider using singleton

        void route(const LogMessage& msg);

    public:
        LogManager() = default;

        // Non-copyable, non-movable
        LogManager(const LogManager& other) = delete;
        LogManager(LogManager&& other) =  delete;
        LogManager& operator = (const LogManager& other) = delete;
        LogManager& operator = (LogManager&& other) = delete;

        void addSink(std::unique_ptr<ILogSink> sink) ;
        void log(const LogMessage& msg);
        void flush();
};