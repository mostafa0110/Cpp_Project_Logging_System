#pragma once
#include "LogTypes.hpp"
#include <string_view>

struct CpuPolicy  // act as Compile-Time Configuration File
{   
    // why static ? access these values globally using the type name
    // without ever creating a variable of type CpuPolicy
    // why constexpr ? values inserted into the assembly code as immediate numbers,
    // not loaded/stored from RAM
    static constexpr TelemetrySrc context = TelemetrySrc::CPU;
    static constexpr std::string_view unit = "%";
    static constexpr float WARNING = 75.0f;
    static constexpr float CRITICAL = 90.0f;

    // constexpr -> compiler will resolve this logic during compilation if the input is known
    static constexpr SeverityLvl inferSeverity(float val) noexcept {
        return (val > CRITICAL) ? SeverityLvl::CRITICAL
            : (val > WARNING)  ? SeverityLvl::WARNING
            :                    SeverityLvl::INFO;
    }
};      // zero overhead for the cpu on runtime


struct GpuPolicy
{
    static constexpr TelemetrySrc context = TelemetrySrc::GPU;
    static constexpr std::string_view unit = "C";
    static constexpr float WARNING = 75.0f;
    static constexpr float CRITICAL = 90.0f;

    static constexpr SeverityLvl inferSeverity(float val) noexcept {
        return (val > CRITICAL) ? SeverityLvl::CRITICAL
            : (val > WARNING)  ? SeverityLvl::WARNING
            :                    SeverityLvl::INFO;
    }
};

struct RamPolicy
{
    static constexpr TelemetrySrc context = TelemetrySrc::RAM;
    static constexpr std::string_view unit = "GB";
    static constexpr float WARNING = 12.0f;
    static constexpr float CRITICAL = 15.0f;

    static constexpr SeverityLvl inferSeverity(float val) noexcept {
        return (val > CRITICAL) ? SeverityLvl::CRITICAL
            : (val > WARNING)  ? SeverityLvl::WARNING
            :                    SeverityLvl::INFO;
    }
};
