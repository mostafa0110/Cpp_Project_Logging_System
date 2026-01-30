#include "LogManagerBuilder.hpp"
#include "ConsoleSinkImpl.hpp"
#include "FileSinkImpl.hpp"
#include <stdexcept>

LogManagerBuilder &LogManagerBuilder::withConsoleSink()
{
    sinks.push_back(std::make_shared<ConsoleSinkImpl>());
    return *this;
}

LogManagerBuilder &LogManagerBuilder::withFileSink(const std::string &filepath)
{
    if (filepath.empty())
    {
        errors.push_back(BuilderError::EMPTY_FILEPATH);
        return *this;
    }
    sinks.push_back(std::make_shared<FileSinkImpl>(filepath));
    return *this;
}

LogManagerBuilder &LogManagerBuilder::withSink(std::shared_ptr<ILogSink> sink)
{
    if (!sink)
    {
        errors.push_back(BuilderError::NULL_SINK);
        return *this;
    }
    sinks.push_back(std::move(sink));
    return *this;
}

LogManagerBuilder &LogManagerBuilder::withSink(LogSinkType type, const std::string &config)
{
    auto result = LogSinkFactory::create(type, config);
    if (result)
    {
        sinks.push_back(std::move(result.value()));
    }
    else
    {
        errors.push_back(BuilderError::SINK_CREATION_FAILED);
    }
    return *this;
}

LogManagerBuilder &LogManagerBuilder::withBufferSize(std::size_t size)
{
    if (size == 0)
    {
        errors.push_back(BuilderError::INVALID_BUFFER_SIZE);
        return *this;
    }
    bufferSize = size;
    return *this;
}

LogManagerBuilder &LogManagerBuilder::withthreadPoolSize(std::size_t size)
{
    if (size == 0)
    {
        errors.push_back(BuilderError::INVALID_THREADPOOL_SIZE);
        return *this;
    }
    threadPoolSize = size;
    return *this;
}

std::unique_ptr<LogManager> LogManagerBuilder::build()
{
    auto result = tryBuild();
    if (!result)
    {
        throw std::runtime_error("LogManagerBuilder::build() failed");
    }
    return std::move(result.value());
}

std::expected<std::unique_ptr<LogManager>, BuilderError> LogManagerBuilder::tryBuild()
{
    if (!errors.empty())
    {
        return std::unexpected(errors.front());
    }

    if (sinks.empty())
    {
        return std::unexpected(BuilderError::NO_SINKS_CONFIGURED);
    }

    auto manager = std::make_unique<LogManager>(bufferSize, threadPoolSize);

    for (auto &sink : sinks)
    {
        manager->addSink(std::move(sink));
    }

    return manager;
}

LogManagerBuilder &LogManagerBuilder::reset()
{
    sinks.clear();
    bufferSize = 100;
    errors.clear();
    return *this;
}