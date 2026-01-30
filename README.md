# C++ Logging System

A modern, modular logging library for C++23 featuring policy-based design, design patterns, thread pool, and zero-cost abstractions.

## Features

- **Thread Pool** - Asynchronous log writing with configurable worker threads
- **Thread-Safe Components** - Mutex-protected sinks and ring buffer
- **Policy-Based Log Formatting** - Compile-time configuration for different telemetry sources (CPU, GPU, RAM)
- **Multiple Sink Support** - Console and File outputs with thread-safe writes
- **Builder Pattern** - Fluent API for LogManager construction
- **Factory Pattern** - Centralized sink creation with error handling
- **Ring Buffer** - Thread-safe circular buffer with blocking/non-blocking operations
- **Type-Safe Enums** - `LogSinkType`, `SeverityLvl`, `TelemetrySrc`
- **Modern Error Handling** - Uses `std::expected` and `std::optional`
- **RAII Wrappers** - `SafeFile`, `SafeSocket` for resource management

## Architecture

```
┌─────────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  TelemetrySource    │────▶│   LogFormatter   │────▶│   LogManager    │
│  (File/Socket)      │     │  (Policy-based)  │     │   (Builder)     │
└─────────────────────┘     └──────────────────┘     └─────────────────┘
                                    │                        │
                                    ▼                        ▼
                            ┌──────────────┐         ┌──────────────┐
                            │  LogMessage  │         │  RingBuffer  │
                            │              │         │ (thread-safe)│
                            └──────────────┘         └──────────────┘
                                                            │
                                                            ▼
                                                    ┌──────────────┐
                                                    │  ThreadPool  │
                                                    │  (N workers) │
                                                    └──────────────┘
                                                            │
                                    ┌───────────────────────┴───────────────────────┐
                                    ▼                                               ▼
                            ┌──────────────┐                                ┌──────────────┐
                            │ ConsoleSink  │                                │   FileSink   │
                            │ (mutex-safe) │                                │ (mutex-safe) │
                            └──────────────┘                                └──────────────┘
```

## Project Structure

```
Cpp_Project_Logging_System/
├── CMakeLists.txt
├── README.md
├── app/
│   ├── CMakeLists.txt
│   └── src/
│       └── main.cpp                # Demo application (Linux telemetry)
└── loggingLib/
    ├── CMakeLists.txt
    ├── inc/
    │   ├── ILogSink.hpp            # Sink interface
    │   ├── ITelemetrySource.hpp    # Telemetry source interface
    │   ├── ConsoleSinkImpl.hpp     # Thread-safe console sink
    │   ├── FileSinkImpl.hpp        # Thread-safe file sink
    │   ├── FileTelemetrySourceImpl.hpp
    │   ├── SocketTelemetrySourceImpl.hpp
    │   ├── LogFormatter.hpp        # Policy-based formatter
    │   ├── LogManager.hpp          # Central log manager
    │   ├── LogManagerBuilder.hpp   # Builder pattern
    │   ├── LogMessage.hpp          # Log message structure
    │   ├── LogPolicies.hpp         # CpuPolicy, GpuPolicy, RamPolicy
    │   ├── LogSinkFactory.hpp      # Factory pattern
    │   ├── LogTypes.hpp            # Enums
    │   ├── RingBuffer.hpp          # Thread-safe circular buffer
    │   ├── ThreadPool.hpp          # Worker thread pool
    │   ├── SafeFile.hpp            # RAII file wrapper
    │   └── SafeSocket.hpp          # RAII socket wrapper
    └── src/
        └── *.cpp
```

## Requirements

- **C++23** (for `std::expected`, concepts)
- **CMake 3.14+**
- **Linux** (uses `/proc/stat`, `/proc/meminfo` for telemetry demo)
- **GCC 13+** or **Clang 16+** (C++23 support)

## Building

```bash
mkdir build && cd build
cmake .. -G "Unix Makefiles"
cmake --build .
```

## Usage

### Basic Usage with Thread Pool

```cpp
#include "LogManagerBuilder.hpp"
#include "LogFormatter.hpp"
#include "LogPolicies.hpp"

int main() {
    auto result = LogManagerBuilder()
        .withConsoleSink()
        .withFileSink("app.log")
        .withBufferSize(100)
        .withthreadPoolSize(4)  // 4 worker threads
        .tryBuild();

    if (!result) {
        return 1;
    }

    auto logger = std::move(result.value());

    LogFormatter<CpuPolicy> cpuFormatter;
    if (auto msg = cpuFormatter.formatDataToLogMsg("75.5")) {
        logger->log(msg.value());  // Buffers message
    }

    logger->flush();  // Dispatches to thread pool
    return 0;
}
```

## Thread Safety

| Component | Thread Safety | Mechanism |
|-----------|---------------|-----------|
| `ThreadPool` | ✅ Safe | Mutex + condition variable |
| `RingBuffer` | ✅ Safe | Mutex + condition variables |
| `ConsoleSinkImpl` | ✅ Safe | Static mutex (one `std::cout`) |
| `FileSinkImpl` | ✅ Safe | Per-instance mutex |
| `LogManager` | ✅ Safe | Thread-safe buffer + shared_ptr sinks |

## Design Patterns

| Pattern | Implementation | Purpose |
|---------|----------------|---------|
| **Builder** | `LogManagerBuilder` | Step-by-step LogManager construction |
| **Factory** | `LogSinkFactory` | Centralized sink creation |
| **Strategy** | `ILogSink` interface | Interchangeable output destinations |
| **Policy** | `LogFormatter<Policy>` | Compile-time behavior configuration |
| **RAII** | `SafeFile`, `SafeSocket` | Resource management |
| **Thread Pool** | `ThreadPool` | Async log writing |

## Ring Buffer API

```cpp
RingBuffer<LogMessage> buffer(100);

// Non-blocking (returns immediately)
buffer.tryPush(msg);           // Returns false if full
auto result = buffer.tryPop(); // Returns std::nullopt if empty

// Blocking (waits until space/data available)
buffer.push(msg);              // Blocks if full
auto msg = buffer.pop();       // Blocks if empty

// Queries
buffer.isEmpty();
buffer.isFull();
buffer.count();
buffer.capacity();
```

## Error Handling

Uses `std::expected` for error handling without exceptions:

```cpp
std::expected<std::shared_ptr<ILogSink>, SinkCreationError>  // Factory
std::expected<std::unique_ptr<LogManager>, BuilderError>     // Builder
std::optional<LogMessage>                                     // Formatter
```

## License

MIT License