#pragma once

#include "interfaces/ITelemetrySource.hpp"
#include "utils/SafeFile.hpp"

class FileTelemetrySourceImpl : public ITelemetrySource {
private:
    std::string filePath;
    SafeFile file;

public:
    FileTelemetrySourceImpl(const std::string& path);
    bool openSource() override;
    bool readSource(std::string& out) override;
};
