#include "LogMessage.hpp"

LogMessage::LogMessage( std::string name,
                        std::string timeStamp,
                        std::string context,
                        std::string severity,
                        std::string payload)
                        :name(std::move(name)),
                         timeStamp(std::move(timeStamp)),
                         context(std::move(context)),
                         severity(std::move(severity)),
                         payload(std::move(payload))  {}


std::ostream& operator << ( std::ostream& os ,const LogMessage& msg){
    os << "[" << msg.name << "] "
       << "[" << msg.timeStamp << "] "
       << "[" << msg.context << "] "
       << "[" << msg.severity << "] "
       << msg.payload;

    return os;
}