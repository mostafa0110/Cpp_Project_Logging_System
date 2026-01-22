#pragma once

#include "ILogSink.hpp"

class ConsoleSinkImpl : public ILogSink
{
public:
    ConsoleSinkImpl() = default;
    ~ConsoleSinkImpl() override = default;

    ConsoleSinkImpl(const ConsoleSinkImpl &) = default;
    ConsoleSinkImpl &operator=(const ConsoleSinkImpl &) = default;
    ConsoleSinkImpl(ConsoleSinkImpl &&) = default;
    ConsoleSinkImpl &operator=(ConsoleSinkImpl &&) = default;

    void write(const LogMessage &msg) override;
};