#include "SafeSocket.hpp"
#include <cstring>
#include <utility>

SafeSocket::SafeSocket() : sockfd(-1) {}

SafeSocket::SafeSocket(int type) : sockfd(::socket(AF_UNIX, type, 0)) {}

SafeSocket::~SafeSocket() {
    close();
}

SafeSocket::SafeSocket(SafeSocket&& other) noexcept : sockfd(other.sockfd) {
    other.sockfd = -1;
}

SafeSocket& SafeSocket::operator=(SafeSocket&& other) noexcept {
    if (this != &other) {
        close();
        sockfd = other.sockfd;
        other.sockfd = -1;
    }
    return *this;
}

bool SafeSocket::isValid() const {
    return sockfd >= 0;
}

bool SafeSocket::create(int type) {
    close();
    sockfd = ::socket(AF_UNIX, type, 0);
    return isValid();
}

bool SafeSocket::connect(const std::string& socketPath) {
    if (!isValid()) return false;

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    return ::connect(sockfd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0;
}

bool SafeSocket::readString(std::string& out, size_t maxSize) const {
    if (!isValid()) return false;

    out.resize(maxSize);
    ssize_t bytesRead = ::read(sockfd, &out[0], maxSize);
    
    if (bytesRead < 0) {
        out.clear();
        return false;
    }
    out.resize(static_cast<size_t>(bytesRead));
    return true;
}

void SafeSocket::close() {
    if (isValid()) {
        ::close(sockfd);
        sockfd = -1;
    }
}
