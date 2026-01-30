#pragma once

#include "ILogSink.hpp"
#include <fstream>
#include <string>
#include <mutex>

class FileSinkImpl : public ILogSink
{
private:
    std::ofstream file;
    std::mutex writeMutex;

public:
    FileSinkImpl() = delete;
    explicit FileSinkImpl(const std::string &path);
    ~FileSinkImpl() override = default;

    // Non-copyable, non-movable (owns file handle and mutex)
    FileSinkImpl(const FileSinkImpl &) = delete;
    FileSinkImpl &operator=(const FileSinkImpl &) = delete;
    FileSinkImpl(FileSinkImpl &&) = delete;
    FileSinkImpl &operator=(FileSinkImpl &&) = delete;

    void write(const LogMessage &msg) override;
    [[nodiscard]] bool isOpen() const noexcept;
};