#include "LogManagerBuilder.hpp"
#include "LogFormatter.hpp"
#include "LogPolicies.hpp"
#include "FileTelemetrySourceImpl.hpp"

#include <iostream>
#include <thread>
#include <chrono>

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
        std::cerr << "Failed to create LogManager: " << toString(result.error()) << '\n';
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
            // Extract CPU usage percentage from /proc/stat (simplified)
            // In real use, you'd parse the data properly
            if (auto msg = cpuFormatter.formatDataToLogMsg("65.5"))
            { // Simulated value
                logger->log(msg.value());
            }
        }

        // Read and log RAM telemetry
        if (memSource.readSource(rawData))
        {
            // Extract RAM usage from /proc/meminfo (simplified)
            if (auto msg = ramFormatter.formatDataToLogMsg("10.2"))
            { // Simulated value in GB
                logger->log(msg.value());
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