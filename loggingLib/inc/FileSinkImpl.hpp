#pragma once

#include "ILogSink.hpp"
#include <fstream>



class FileSinkImpl : public ILogSink{
    private:
        std::ofstream file;

    public:
        FileSinkImpl() = delete;
        FileSinkImpl(const std::string& path);
        void write(const LogMessage & msg) override;
};