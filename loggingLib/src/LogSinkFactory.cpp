#include "LogSinkFactory.hpp"
#include "FileSinkImpl.hpp"
#include "ConsoleSinkImpl.hpp"

std::expected<std::unique_ptr<ILogSink>, SinkCreationError> LogSinkFactory::create(LogSinkType type, const std::string &config)
{
    switch (type)
    {
    case LogSinkType::CONSOLE:
        return std::make_unique<ConsoleSinkImpl>();

    case LogSinkType::FILE:
        if (config.empty())
        {
            return std::unexpected(SinkCreationError::MISSING_FILEPATH);
        }
        return std::make_unique<FileSinkImpl>(config);

        /*case LogSinkType::SOCKET:
            if (config.empty())
            {
                return std::unexpected(SinkCreationError::MISSING_SOCKET_ADDRESS);
            }
            return */

    default:
        return std::unexpected(SinkCreationError::UNKNOWN_SINK_TYPE);
    }
}
