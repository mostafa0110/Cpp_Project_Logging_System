#include "LogSinkFactory.hpp"
#include "FileSinkImpl.hpp"
#include "ConsoleSinkImpl.hpp"

std::expected<std::shared_ptr<ILogSink>, SinkCreationError> LogSinkFactory::create(LogSinkType type, const std::string &config)
{
    switch (type)
    {
    case LogSinkType::CONSOLE:
        return std::make_shared<ConsoleSinkImpl>();

    case LogSinkType::FILE:
        if (config.empty())
        {
            return std::unexpected(SinkCreationError::MISSING_FILEPATH);
        }
        return std::make_shared<FileSinkImpl>(config);

    default:
        return std::unexpected(SinkCreationError::UNKNOWN_SINK_TYPE);
    }
}
