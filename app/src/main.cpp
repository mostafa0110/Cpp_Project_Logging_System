#include "LogManager.hpp"
#include "ConsoleSinkImpl.hpp"
#include "FileSinkImpl.hpp"
#include "FileTelemetrySourceImpl.hpp"

#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <ctime>
#include <iomanip>

std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time));
    return std::string(buffer);
}

std::string parseCpuInfo(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::getline(iss, line);

    std::istringstream lineStream(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    
    if (!(lineStream >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)) {
        return "Failed to parse CPU data";
    }

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;

    std::ostringstream result;
    result << "CPU - User: " << user << " | System: " << system << " | Idle: " << idle 
           << " | Total: " << total;
    return result.str();
}

std::string parseMemInfo(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    unsigned long long memTotal = 0, memAvailable = 0;

    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        std::string key;
        unsigned long long value;
        lineStream >> key >> value;
        
        if (key == "MemTotal:") memTotal = value;
        else if (key == "MemAvailable:") memAvailable = value;
    }

    double totalMb = memTotal / 1024.0;
    double usedMb = (memTotal - memAvailable) / 1024.0;
    double percent = 100.0 * (memTotal - memAvailable) / memTotal;

    std::ostringstream result;
    result << std::fixed << std::setprecision(1);
    result << "Memory - Used: " << usedMb << "/" << totalMb << " MB (" << percent << "%)";
    return result.str();
}

int main() {
    LogManager logger;
    logger.addSink(std::make_unique<ConsoleSinkImpl>());
    logger.addSink(std::make_unique<FileSinkImpl>("system_telemetry.log"));

    FileTelemetrySourceImpl cpuSource("/proc/stat");
    FileTelemetrySourceImpl memSource("/proc/meminfo");

    if (!cpuSource.openSource() || !memSource.openSource()) {
        std::cerr << "Failed to open /proc files" << std::endl;
        return 1;
    }

    std::cout << "=== System Telemetry Demo ===" << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::string cpuData, memData;
        std::string timestamp = getCurrentTimestamp();

        if (cpuSource.readSource(cpuData)) {
            logger.log(LogMessage("Telemetry", timestamp, "CPU", "Info", parseCpuInfo(cpuData)));
        }

        if (memSource.readSource(memData)) {
            logger.log(LogMessage("Telemetry", timestamp, "Memory", "Info", parseMemInfo(memData)));
        }

        logger.flush();

        if (i < 4) std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "=== Complete ===" << std::endl;
    return 0;
}