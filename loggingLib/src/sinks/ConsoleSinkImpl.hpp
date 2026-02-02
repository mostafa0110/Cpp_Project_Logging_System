#pragma once

#include "interfaces/ILogSink.hpp"
#include <mutex>

class ConsoleSinkImpl : public ILogSink
{
private:
    static std::mutex coutMutex;

public:
    ConsoleSinkImpl() = default;
    ~ConsoleSinkImpl() override = default;

    ConsoleSinkImpl(const ConsoleSinkImpl &) = default;
    ConsoleSinkImpl &operator=(const ConsoleSinkImpl &) = default;
    ConsoleSinkImpl(ConsoleSinkImpl &&) = default;
    ConsoleSinkImpl &operator=(ConsoleSinkImpl &&) = default;

    void write(const LogMessage &msg) override;
};