# C++ Logging System

A modern, modular logging library for C++23 featuring policy-based design, design patterns, and zero-cost abstractions.

## Features

- **Policy-Based Log Formatting** - Compile-time configuration for different telemetry sources (CPU, GPU, RAM)
- **Multiple Sink Support** - Console, File, and Socket outputs
- **Builder Pattern** - Fluent API for LogManager construction
- **Factory Pattern** - Centralized sink creation with error handling
- **Ring Buffer** - Fixed-size circular buffer with move semantics
- **Type-Safe Enums** - `LogSinkType`, `SeverityLvl`, `TelemetrySrc`
- **Modern Error Handling** - Uses `std::expected` and `std::optional`

## Architecture

```
┌─────────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  TelemetrySource    │────▶│   LogFormatter   │────▶│   LogManager    │
│  (File/Socket)      │     │  (Policy-based)  │     │   (Builder)     │
└─────────────────────┘     └──────────────────┘     └─────────────────┘
                                    │                        │
                                    ▼                        ▼
                            ┌──────────────┐         ┌──────────────┐
                            │  LogMessage  │         │  ILogSink    │
                            │              │         │  (Factory)   │
                            └──────────────┘         └──────────────┘
                                                            │
                                    ┌───────────────────────┼───────────────────────┐
                                    ▼                       ▼                       ▼
                            ┌──────────────┐        ┌──────────────┐        ┌──────────────┐
                            │ ConsoleSink  │        │   FileSink   │        │  SocketSink  │
                            └──────────────┘        └──────────────┘        └──────────────┘
```

## Project Structure

```
Cpp_Project_Logging_System/
├── CMakeLists.txt              # Root CMake configuration
├── README.md
├── app/
│   ├── CMakeLists.txt
│   └── src/
│       └── main.cpp            # Demo application
├── libs/
│   └── magic_enum/             # Enum reflection library
└── loggingLib/
    ├── CMakeLists.txt
    ├── inc/
    │   ├── ILogSink.hpp            # Sink interface
    │   ├── ITelemetrySource.hpp    # Telemetry source interface
    │   ├── ConsoleSinkImpl.hpp     # Console output sink
    │   ├── FileSinkImpl.hpp        # File output sink
    │   ├── FileTelemetrySourceImpl.hpp
    │   ├── SocketTelemetrySourceImpl.hpp
    │   ├── LogFormatter.hpp        # Policy-based formatter (template)
    │   ├── LogManager.hpp          # Central log manager
    │   ├── LogManagerBuilder.hpp   # Builder pattern
    │   ├── LogMessage.hpp          # Log message structure
    │   ├── LogPolicies.hpp         # CpuPolicy, GpuPolicy, RamPolicy
    │   ├── LogSinkFactory.hpp      # Factory pattern
    │   ├── LogTypes.hpp            # Enums + toString helpers
    │   ├── RingBuffer.hpp          # Circular buffer (template)
    │   ├── SafeFile.hpp            # RAII file wrapper
    │   └── SafeSocket.hpp          # RAII socket wrapper
    └── src/
        └── *.cpp                   # Implementations
```

## Requirements

- **C++23** (for `std::expected`, concepts)
- **CMake 3.14+**
- **magic_enum** library (included in libs/)

## Building

```bash
# Clone the repository
git clone <repo-url>
cd Cpp_Project_Logging_System

# Clone magic_enum dependency
git clone https://github.com/Neargye/magic_enum.git libs/magic_enum

# Build
mkdir build && cd build
cmake ..
cmake --build .
```

## Usage

### Basic Usage with Builder

```cpp
#include "LogManagerBuilder.hpp"
#include "LogFormatter.hpp"
#include "LogPolicies.hpp"

int main() {
    // Build LogManager with fluent API
    auto result = LogManagerBuilder()
        .withConsoleSink()
        .withFileSink("app.log")
        .withBufferSize(100)
        .tryBuild();

    if (!result) {
        std::cerr << "Error: " << toString(result.error()) << std::endl;
        return 1;
    }

    auto logger = std::move(result.value());

    // Create formatter with policy
    LogFormatter<CpuPolicy> cpuFormatter;

    // Format and log
    if (auto msg = cpuFormatter.formatDataToLogMsg("75.5")) {
        logger->log(msg.value());
    }

    logger->flush();
    return 0;
}
```

### Using the Factory Directly

```cpp
#include "LogSinkFactory.hpp"

auto sinkResult = LogSinkFactory::create(LogSinkType::FILE, "output.log");
if (sinkResult) {
    auto sink = std::move(sinkResult.value());
    // Use sink...
}
```

### Custom Policy

```cpp
struct MyCustomPolicy {
    static constexpr TelemetrySrc context = TelemetrySrc::CPU;
    static constexpr std::string_view unit = "%";
    static constexpr float WARNING = 80.0f;
    static constexpr float CRITICAL = 95.0f;

    static constexpr SeverityLvl inferSeverity(float val) noexcept {
        return (val > CRITICAL) ? SeverityLvl::CRITICAL
             : (val > WARNING)  ? SeverityLvl::WARNING
             :                    SeverityLvl::INFO;
    }
};

LogFormatter<MyCustomPolicy> formatter;
```

## Design Patterns Used

| Pattern | Implementation | Purpose |
|---------|----------------|---------|
| **Builder** | `LogManagerBuilder` | Step-by-step LogManager construction |
| **Factory** | `LogSinkFactory` | Centralized sink creation |
| **Strategy** | `ILogSink` interface | Interchangeable output destinations |
| **Policy** | `LogFormatter<Policy>` | Compile-time behavior configuration |
| **RAII** | `SafeFile`, `SafeSocket` | Resource management |

## Enums

```cpp
enum class LogSinkType { CONSOLE, FILE, SOCKET };
enum class SeverityLvl { CRITICAL, WARNING, INFO };
enum class TelemetrySrc { CPU, GPU, RAM };
```

All enums have `toString()` functions and `operator<<` overloads for easy output.

## Error Handling

The library uses `std::expected` for error handling without exceptions:

```cpp
// Factory returns expected
std::expected<std::unique_ptr<ILogSink>, SinkCreationError>

// Builder returns expected
std::expected<std::unique_ptr<LogManager>, BuilderError>

// Formatter returns optional
std::optional<LogMessage>
```

## Ring Buffer


```cpp
RingBuffer<LogMessage> buffer(100);  // Capacity of 100

buffer.tryPush(msg);           // Returns false if full
auto result = buffer.tryPop(); // Returns std::nullopt if empty

buffer.isEmpty();
buffer.isFull();
buffer.count();
buffer.capacity();
```

## License

MIT License