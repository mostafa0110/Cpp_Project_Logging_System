#include "LogManagerBuilder.hpp"
#include "LogFormatter.hpp"
#include "LogPolicies.hpp"
#include "FileTelemetrySourceImpl.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>

int main()
{
    // ===== Build LogManager using Builder =====
    auto result = LogManagerBuilder()
                      .withConsoleSink()
                      .withFileSink("system_telemetry.log")
                      .withBufferSize(50)
                      .tryBuild();

    if (!result)
    {
        std::cerr << "Failed to create LogManager: \n" ;
        return 1;
    }

    auto logger = std::move(result.value());

    // ===== Setup Telemetry Sources (Linux /proc files) =====
    FileTelemetrySourceImpl cpuSource("/proc/stat");
    FileTelemetrySourceImpl memSource("/proc/meminfo");

    // ===== Setup Formatters (Policy-based) =====
    LogFormatter<CpuPolicy> cpuFormatter;
    LogFormatter<RamPolicy> ramFormatter;

    // ===== Open Sources =====
    if (!cpuSource.openSource())
    {
        std::cerr << "Failed to open /proc/stat" << std::endl;
        return 1;
    }
    if (!memSource.openSource())
    {
        std::cerr << "Failed to open /proc/meminfo" << std::endl;
        return 1;
    }

    std::cout << "=== System Telemetry Demo ===\n";
    std::cout << "Reading from Linux /proc files...\n\n";

    // ===== Main Loop =====
    for (int i = 0; i < 5; ++i)
    {
        std::string rawData;

        // Read and log CPU telemetry
        if (cpuSource.readSource(rawData))
        {
            // Parse /proc/stat format: "cpu  78412 3040 14706 1944026 ..."
            // Extract the first number (user ticks) after "cpu"
            std::istringstream iss(rawData);
            std::string label;
            long long userTicks;
            
            iss >> label >> userTicks;
            
            if (label == "cpu" && iss)
            {
                if (auto msg = cpuFormatter.formatDataToLogMsg(std::to_string(userTicks)))
                {
                    logger->log(msg.value());
                }
            }
        }

        // Read and log RAM telemetry
        if (memSource.readSource(rawData))
        {
            // Parse /proc/meminfo to find MemAvailable
            // Format: "MemAvailable:   11228316 kB"
            std::istringstream memStream(rawData);
            std::string line;
            
            while (std::getline(memStream, line))
            {
                if (line.find("MemAvailable:") == 0)
                {
                    std::istringstream lineStream(line);
                    std::string label;
                    long long memKB;
                    
                    lineStream >> label >> memKB;
                    
                    if (label == "MemAvailable:" && lineStream)
                    {
                        // Convert KB to GB for display
                        double memGB = memKB / (1024.0 * 1024.0);
                        if (auto msg = ramFormatter.formatDataToLogMsg(std::to_string(memGB)))
                        {
                            logger->log(msg.value());
                        }
                    }
                    break;
                }
            }
        }

        // Flush logs to sinks
        logger->flush();

        if (i < 4)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    std::cout << "\n=== Complete ===\n";
    return 0;
}