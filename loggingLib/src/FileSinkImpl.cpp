#include "FileSinkImpl.hpp"

FileSinkImpl::FileSinkImpl(const std::string &path)
    : file(path, std::ios::app)
{
}

void FileSinkImpl::write(const LogMessage &msg)
{
    if (file.is_open())
    {
        file << msg << std::endl;
    }
}

bool FileSinkImpl::isOpen() const noexcept
{
    return file.is_open();
}