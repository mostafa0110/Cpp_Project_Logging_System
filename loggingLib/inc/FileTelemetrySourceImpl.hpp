#pragma once

#include "ITelemetrySource.hpp"
#include "SafeFile.hpp"

class FileTelemetrySourceImpl : public ITelemetrySource {
private:
    std::string filePath;
    SafeFile file;

public:
    FileTelemetrySourceImpl(const std::string& path);
    bool openSource() override;
    bool readSource(std::string& out) override;
};
