#include "SocketTelemetrySourceImpl.hpp"
#include <sys/socket.h>

SocketTelemetrySourceImpl::SocketTelemetrySourceImpl(const std::string& path)
    : socketPath(path), socket() {}

bool SocketTelemetrySourceImpl::openSource() {
    if (!socket.create(SOCK_STREAM)) {
        return false;
    }
    return socket.connect(socketPath);
}

bool SocketTelemetrySourceImpl::readSource(std::string& out) {
    if (!socket.isValid()) {
        return false;
    }
    return socket.readString(out);
}
