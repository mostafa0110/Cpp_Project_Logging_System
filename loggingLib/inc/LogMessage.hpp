#pragma once

#include <string>
#include <ostream>

class LogMessage {
    private:
        std::string name;
        std::string timeStamp;
        std::string context;
        std::string severity;
        std::string payload;
    
    public:
        LogMessage() = delete;
        LogMessage(     std::string name,
                        std::string timeStamp,
                        std::string context,
                        std::string severity,
                        std::string payload);

        LogMessage(const LogMessage&) = default;
        LogMessage(LogMessage&&) = default;
        LogMessage& operator=(const LogMessage&) = default;
        LogMessage& operator=(LogMessage&&) = default;
        ~LogMessage() = default;

        friend std::ostream& operator << ( std::ostream& os ,const LogMessage& msg);
};