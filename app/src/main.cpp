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
    ThreadPool pool(2);
    // ===== Build LogManager using Builder =====
    auto result = LogManagerBuilder()
                      .withConsoleSink()
                      .withFileSink("system_telemetry.log")
                      .withBufferSize(50)
                      .withthreadPoolSize(3)
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


    for (int i = 0; i < 5; ++i)
    {
        pool.enqueue([&] {
            std::string rawData;
            if (cpuSource.readSource(rawData))
            {
                std::istringstream iss(rawData);
                std::string label;
                long long userTicks;

                iss >> label >> userTicks;
                if (label == "cpu" && iss)
                {
                    if (auto msg = cpuFormatter.formatDataToLogMsg(
                            std::to_string(userTicks)))
                    {
                        logger->log(msg.value());
                    }
                }
            }
        });

        pool.enqueue([&] {
            std::string rawData;
            if (memSource.readSource(rawData))
            {
                std::istringstream memStream(rawData);
                std::string line;

                while (std::getline(memStream, line))
                {
                    if (line.rfind("MemAvailable:", 0) == 0)
                    {
                        std::istringstream lineStream(line);
                        std::string label;
                        long long memKB;

                        lineStream >> label >> memKB;
                        double memGB = memKB / (1024.0 * 1024.0);

                        if (auto msg = ramFormatter.formatDataToLogMsg(
                                std::to_string(memGB)))
                        {
                            logger->log(msg.value());
                        }
                        break;
                    }
                }
            }
        });

        std::this_thread::sleep_for(std::chrono::seconds(1));
        logger->flush();
    }

    std::cout << "\n=== Complete ===\n";
    return 0;
}