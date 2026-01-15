#include "FileTelemetrySourceImpl.hpp"
#include <fcntl.h>

FileTelemetrySourceImpl::FileTelemetrySourceImpl(const std::string& path)
    : filePath(path), file() {}

bool FileTelemetrySourceImpl::openSource() {
    return file.open(filePath, O_RDONLY);
}

bool FileTelemetrySourceImpl::readSource(std::string& out) {
    if (!file.isValid()) {
        return false;
    }
    return file.readAll(out);
}
