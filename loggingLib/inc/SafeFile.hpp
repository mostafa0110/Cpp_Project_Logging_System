#pragma once

#include <unistd.h>
#include <fcntl.h>
#include <string>

class SafeFile {
private:
    int fd;

public:
    SafeFile();
    SafeFile(const std::string& path, int flags, mode_t mode = 0644);
    ~SafeFile();

    SafeFile(const SafeFile&) = delete;
    SafeFile& operator=(const SafeFile&) = delete;

    SafeFile(SafeFile&& other) noexcept;
    SafeFile& operator=(SafeFile&& other) noexcept;

    bool isValid() const;
    bool open(const std::string& path, int flags, mode_t mode = 0644);
    bool readAll(std::string& out) const;
    void close();
};
