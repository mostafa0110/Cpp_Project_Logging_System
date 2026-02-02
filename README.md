# C++ Logging System

A modern, modular logging library for C++23 featuring policy-based design, design patterns, thread pool, vsomeip integration, and zero-cost abstractions.

## Documentation

| Document | Description |
|----------|-------------|
| [README.md](README.md) | Overview, quick start, CMake targets |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Design patterns, threading model, data flow |
| [docs/SOMEIP.md](docs/SOMEIP.md) | vsomeip configuration, testing, troubleshooting |
| [docs/API.md](docs/API.md) | Complete class and method reference |

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
- **SomeIP Telemetry** - Remote telemetry via vsomeip protocol

## Architecture

```
┌─────────────────────┐     ┌──────────────────┐     ┌─────────────────┐
│  TelemetrySource    │────▶│   LogFormatter   │────▶│   LogManager    │
│  (File/Socket/      │     │  (Policy-based)  │     │   (Builder)     │
│   SomeIP)           │     └──────────────────┘     └─────────────────┘
└─────────────────────┘             │                        │
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
├── CMakeLists.txt              # Root CMake with vsomeip config
├── README.md
├── config/                     # vsomeip JSON configurations
│   ├── vsomeip-client.json     # Client-side config
│   └── vsomeip-server.json     # Server-side config
├── app/
│   ├── CMakeLists.txt
│   └── src/
│       └── main.cpp            # Demo app (Linux + SomeIP telemetry)
├── loggingLib/
│   ├── CMakeLists.txt
│   ├── include/                # Public headers
│   │   ├── interfaces/         # ILogSink, ITelemetrySource
│   │   ├── LogFormatter.hpp
│   │   ├── LogManager.hpp
│   │   ├── LogManagerBuilder.hpp
│   │   ├── LogMessage.hpp
│   │   ├── LogPolicies.hpp
│   │   ├── LogSinkFactory.hpp
│   │   └── LogTypes.hpp
│   └── src/                    # Private implementation
│       ├── core/               # LogManager, LogMessage, Factory
│       ├── sinks/              # Console/File sink implementations
│       ├── sources/            # File, Socket, SomeIP adapters
│       ├── concurrency/        # ThreadPool, RingBuffer
│       └── utils/              # SafeFile, SafeSocket RAII wrappers
└── test/
    ├── CMakeLists.txt
    ├── SomeIPTestServer.hpp    # Mock vsomeip server
    ├── someip_test_main.cpp    # Test server executable
    └── someip_test_client.cpp  # Test client executable
```

## Requirements

- **C++23** (for `std::expected`, concepts)
- **CMake 3.14+**
- **Linux** (uses `/proc/stat`, `/proc/meminfo` for telemetry demo)
- **GCC 13+** or **Clang 16+** (C++23 support)
- **vsomeip 3.x** (for SomeIP telemetry)

## Building

```bash
mkdir build && cd build
cmake .. -G "Unix Makefiles"
cmake --build .
```

### Build Outputs

| Target | Description |
|--------|-------------|
| `app` | Main demo application |
| `logging` | Static logging library |
| `someip_test_server` | SomeIP test server |
| `someip_test_client` | SomeIP test client |

## CMake Custom Targets

The project provides convenient run targets:

| Target | Command | Description |
|--------|---------|-------------|
| `run_app_someip` | `make run_app_someip` | Run main app with SomeIP client config |
| `run_test_server` | `make run_test_server` | Start SomeIP test server |
| `run_test_client` | `make run_test_client` | Start SomeIP test client |

---

## Building with Bazel

The project also supports Bazel as an alternative build system.

### Requirements

- **Bazel 7.0+** (with bzlmod support)
- **vsomeip 3.x** installed at `/usr/local`

### Bazel Project Structure

```
MODULE.bazel          # Project deps (magic_enum, vsomeip)
.bazelrc              # Build config (C++23)
loggingLib/BUILD      # Library target
app/BUILD             # App binary
test/BUILD            # Test binaries
```

### Build Commands

```bash
# Build logging library
bazel build //loggingLib:logging

# Build main app
bazel build //app:app

# Build test server
bazel build //test:someip_test_server

# Build test client
bazel build //test:someip_test_client

# Build all
bazel build //...
```

### Run Commands

```bash
# Run main app (VSOMEIP_CONFIGURATION auto-set)
bazel run //app:app

# Run test server (Terminal 1)
bazel run //test:someip_test_server

# Run test client (Terminal 2)
bazel run //test:someip_test_client
```

### Bazel Targets Summary

| Target | Description |
|--------|-------------|
| `//loggingLib:logging` | Core library with vsomeip |
| `//app:app` | Main demo application |
| `//test:someip_test_server` | Mock vsomeip server |
| `//test:someip_test_client` | Test client |

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

### Using SomeIP Telemetry Source

```cpp
#include "sources/SomeIPTelemetryAdapter.hpp"

SomeIPTelemetryAdapter someipSource;

// Initialize (connects to vsomeip server)
if (someipSource.openSource()) {
    std::string data;
    if (someipSource.readSource(data)) {
        std::cout << "Remote load: " << data << "%" << std::endl;
    }
}
```

## Testing SomeIP

The project includes a complete vsomeip test setup:

### 1. Start the Test Server

```bash
cd build
make run_test_server
```

The server will output:
```
=== SomeIP Telemetry Test Server ===
Service ID: 0x1234
Instance ID: 0x5678
Method ID: 0x0001

Server is running. Press Ctrl+C to stop.
Responding with fixed load value: 75.5%
```

### 2. Run the Test Client (in another terminal)

```bash
cd build
make run_test_client
```

Expected output:
```
=== SomeIP Telemetry Test Client ===
Connecting to Service ID: 0x1234, Instance: 0x5678

Client initialized. Waiting for server...
[Request #1] Received load: 75.5%
[Request #2] Received load: 75.5%
...
```

### 3. Run Main App with SomeIP

```bash
# Terminal 1: Start test server
make run_test_server

# Terminal 2: Run app
make run_app_someip
```

## SomeIP Configuration

Configuration files in `config/`:

### vsomeip-client.json
```json
{
    "unicast": "127.0.0.1",
    "applications": [{ "name": "TelemetryClient", "id": "0x1111" }],
    "service-discovery": { "enable": true, "multicast": "224.224.224.245", "port": 30490 }
}
```

### vsomeip-server.json
```json
{
    "unicast": "127.0.0.1",
    "applications": [{ "name": "TelemetryServer", "id": "0x2222" }],
    "routing": "TelemetryServer",
    "services": [{ "service": "0x1234", "instance": "0x5678", "reliable": { "port": 30509 } }]
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
| `SomeIPTelemetrySourceImpl` | ✅ Safe | Mutex + atomic flags |

## Design Patterns

| Pattern | Implementation | Purpose |
|---------|----------------|---------|
| **Builder** | `LogManagerBuilder` | Step-by-step LogManager construction |
| **Factory** | `LogSinkFactory` | Centralized sink creation |
| **Strategy** | `ILogSink` interface | Interchangeable output destinations |
| **Policy** | `LogFormatter<Policy>` | Compile-time behavior configuration |
| **Adapter** | `SomeIPTelemetryAdapter` | Adapts vsomeip to ITelemetrySource |
| **Singleton** | `SomeIPTelemetrySourceImpl` | Single vsomeip application instance |
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