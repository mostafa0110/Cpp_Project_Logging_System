#include "SafeFile.hpp"
#include <utility>

SafeFile::SafeFile() : fd(-1) {}

SafeFile::SafeFile(const std::string& path, int flags, mode_t mode)
    : fd(::open(path.c_str(), flags, mode)) {}

SafeFile::~SafeFile() {
    close();
}

SafeFile::SafeFile(SafeFile&& other) noexcept : fd(other.fd) {
    other.fd = -1;
}

SafeFile& SafeFile::operator=(SafeFile&& other) noexcept {
    if (this != &other) {
        close();
        fd = other.fd;
        other.fd = -1;
    }
    return *this;
}

bool SafeFile::isValid() const {
    return fd >= 0;
}

bool SafeFile::open(const std::string& path, int flags, mode_t mode) {
    close();
    fd = ::open(path.c_str(), flags, mode);
    return isValid();
}

bool SafeFile::readAll(std::string& out) const {
    if (!isValid()) return false;

    out.clear();
    char buffer[4096];
    ssize_t bytesRead;

    ::lseek(fd, 0, SEEK_SET);
    while ((bytesRead = ::read(fd, buffer, sizeof(buffer))) > 0) {
        out.append(buffer, bytesRead);
    }
    return bytesRead >= 0;
}

void SafeFile::close() {
    if (isValid()) {
        ::close(fd);
        fd = -1;
    }
}
