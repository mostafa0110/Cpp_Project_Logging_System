#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>

class SafeSocket {
private:
    int sockfd;

public:
    SafeSocket();
    SafeSocket(int type);
    ~SafeSocket();

    SafeSocket(const SafeSocket&) = delete;
    SafeSocket& operator=(const SafeSocket&) = delete;

    SafeSocket(SafeSocket&& other) noexcept;
    SafeSocket& operator=(SafeSocket&& other) noexcept;

    bool isValid() const;
    bool create(int type);
    bool connect(const std::string& socketPath);
    bool readString(std::string& out, size_t maxSize = 4096) const;
    void close();
};
