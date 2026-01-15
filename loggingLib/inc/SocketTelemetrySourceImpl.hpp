#pragma once

#include "ITelemetrySource.hpp"
#include "SafeSocket.hpp"

class SocketTelemetrySourceImpl : public ITelemetrySource {
private:
    std::string socketPath;
    SafeSocket socket;

public:
    SocketTelemetrySourceImpl(const std::string& path);
    bool openSource() override;
    bool readSource(std::string& out) override;
};
