#include "LogManager.hpp"
#include "ConsoleSinkImpl.hpp"
#include "FileSinkImpl.hpp"


int main() {
    LogManager logger;

    logger.addSink(std::make_unique<ConsoleSinkImpl>());
    logger.addSink(std::make_unique<FileSinkImpl>("app.log"));

    logger.log(LogMessage("MyApp","1", "Init", "Info", "Application started"));
    logger.log(LogMessage("MyApp","2", "Core", "Warning", "Low memory"));
    logger.log(LogMessage("MyApp","3", "Core", "Error", "Fatal error"));

    logger.flush();
    return 0;
}