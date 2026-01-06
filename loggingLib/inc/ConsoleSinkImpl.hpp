#pragma once

#include "ILogSink.hpp"



class ConsoleSinkImpl : public ILogSink{
    public:
        void write(const LogMessage & msg) override;
};