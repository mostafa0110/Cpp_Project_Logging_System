#pragma once

#include "interfaces/ITelemetrySource.hpp"
#include "SomeIPTelemetrySourceImpl.hpp"
#include <sstream>

class SomeIPTelemetryAdapter : public ITelemetrySource {
public:
    SomeIPTelemetryAdapter() = default;
    ~SomeIPTelemetryAdapter() override = default;

    SomeIPTelemetryAdapter(const SomeIPTelemetryAdapter&) = default;
    SomeIPTelemetryAdapter& operator=(const SomeIPTelemetryAdapter&) = default;
    SomeIPTelemetryAdapter(SomeIPTelemetryAdapter&&) = default;
    SomeIPTelemetryAdapter& operator=(SomeIPTelemetryAdapter&&) = default;

    bool openSource() override {
        return SomeIPTelemetrySourceImpl::getInstance().init();
    }

    bool readSource(std::string& out) override {
        auto& client = SomeIPTelemetrySourceImpl::getInstance();
        
        if (!client.isAvailable()) {
            return false;
        }
        
        float loadPercentage = 0.0f;
        if (client.requestLoadData(loadPercentage)) {
            std::ostringstream oss;
            oss << loadPercentage;
            out = oss.str();
            return true;
        }
        
        return false;
    }
};
