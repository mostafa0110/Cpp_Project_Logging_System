# API Reference

Complete API documentation for the C++ Logging System.

## Table of Contents

1. [Core Classes](#core-classes)
2. [Interfaces](#interfaces)
3. [Telemetry Sources](#telemetry-sources)
4. [Sinks](#sinks)
5. [Concurrency](#concurrency)
6. [Utilities](#utilities)
7. [Types and Enums](#types-and-enums)

---

## Core Classes

### LogManager

Central manager for buffering and dispatching log messages.

**Header**: `include/LogManager.hpp`

```cpp
class LogManager {
public:
    explicit LogManager(
        std::size_t bufferCapacity = 100,
        std::size_t numThreads = 4
    );
    
    void addSink(std::shared_ptr<ILogSink> sink);
    void log(const LogMessage& msg);
    void flush();
};
```

| Method | Description | Thread-Safe |
|--------|-------------|-------------|
| `addSink(sink)` | Registers a sink for output | Yes |
| `log(msg)` | Pushes message to internal buffer | Yes |
| `flush()` | Dispatches all buffered messages to thread pool | Yes |

**Example**:
```cpp
LogManager manager(100, 4);  // 100 buffer, 4 threads
manager.addSink(consoleSink);
manager.log(message);
manager.flush();
```

---

### LogManagerBuilder

Fluent builder for LogManager construction.

**Header**: `include/LogManagerBuilder.hpp`

```cpp
class LogManagerBuilder {
public:
    LogManagerBuilder& withConsoleSink();
    LogManagerBuilder& withFileSink(const std::string& filepath);
    LogManagerBuilder& withSink(std::shared_ptr<ILogSink> sink);
    LogManagerBuilder& withSink(LogSinkType type, const std::string& config = "");
    LogManagerBuilder& withBufferSize(std::size_t size);
    LogManagerBuilder& withthreadPoolSize(std::size_t size);
    
    [[nodiscard]] std::unique_ptr<LogManager> build();
    [[nodiscard]] std::expected<std::unique_ptr<LogManager>, BuilderError> tryBuild();
    
    LogManagerBuilder& reset();
};
```

**Example**:
```cpp
auto result = LogManagerBuilder()
    .withConsoleSink()
    .withFileSink("app.log")
    .withBufferSize(50)
    .withthreadPoolSize(3)
    .tryBuild();

if (result) {
    auto logger = std::move(result.value());
}
```

---

### LogMessage

Value type representing a log entry.

**Header**: `include/LogMessage.hpp`

```cpp
struct LogMessage {
    TelemetrySrc source;
    SeverityLvl severity;
    std::string timestamp;
    std::string description;
    
    std::string toString() const;
};
```

**Example**:
```cpp
LogMessage msg{
    TelemetrySrc::CPU,
    SeverityLvl::WARNING,
    "2024-01-15 10:30:00",
    "CPU: 75.5% | Warning: Above normal"
};
```

---

### LogFormatter<Policy>

Policy-based formatter that converts raw data to LogMessage.

**Header**: `include/LogFormatter.hpp`

```cpp
template <typename Policy>
class LogFormatter {
public:
    [[nodiscard]] std::optional<LogMessage> formatDataToLogMsg(const std::string& raw);
};
```

**Policy Requirements**:
```cpp
struct Policy {
    static constexpr TelemetrySrc context;      // Source type
    static constexpr float WARNING;              // Warning threshold
    static constexpr float CRITICAL;             // Critical threshold
    static constexpr std::string_view unit;      // Display unit
    
    static SeverityLvl inferSeverity(float val); // Severity logic
};
```

**Available Policies**: `CpuPolicy`, `RamPolicy`, `GpuPolicy`

**Example**:
```cpp
LogFormatter<CpuPolicy> formatter;
auto msg = formatter.formatDataToLogMsg("75.5");
if (msg) {
    logger->log(msg.value());
}
```

---

### LogSinkFactory

Factory for creating sink instances.

**Header**: `include/LogSinkFactory.hpp`

```cpp
class LogSinkFactory {
public:
    static std::expected<std::shared_ptr<ILogSink>, SinkCreationError>
        createSink(LogSinkType type, const std::string& config = "");
};
```

**Example**:
```cpp
auto result = LogSinkFactory::createSink(LogSinkType::File, "output.log");
if (result) {
    auto sink = result.value();
}
```

---

## Interfaces

### ILogSink

Abstract interface for log output destinations.

**Header**: `include/interfaces/ILogSink.hpp`

```cpp
class ILogSink {
public:
    virtual ~ILogSink() noexcept = default;
    virtual void write(const LogMessage& msg) = 0;
};
```

**Implementations**: `ConsoleSinkImpl`, `FileSinkImpl`

---

### ITelemetrySource

Abstract interface for telemetry data sources.

**Header**: `include/interfaces/ITelemetrySource.hpp`

```cpp
class ITelemetrySource {
public:
    virtual bool openSource() = 0;
    virtual bool readSource(std::string& out) = 0;
    virtual ~ITelemetrySource() = default;
};
```

**Implementations**: `FileTelemetrySourceImpl`, `SocketTelemetrySourceImpl`, `SomeIPTelemetryAdapter`

---

## Telemetry Sources

### FileTelemetrySourceImpl

Reads telemetry from files (e.g., Linux `/proc` files).

**Header**: `src/sources/FileTelemetrySourceImpl.hpp`

```cpp
class FileTelemetrySourceImpl : public ITelemetrySource {
public:
    explicit FileTelemetrySourceImpl(const std::string& path);
    bool openSource() override;
    bool readSource(std::string& out) override;
};
```

**Example**:
```cpp
FileTelemetrySourceImpl cpuSource("/proc/stat");
cpuSource.openSource();

std::string data;
if (cpuSource.readSource(data)) {
    // Parse CPU data
}
```

---

### SocketTelemetrySourceImpl

Reads telemetry from network sockets.

**Header**: `src/sources/SocketTelemetrySourceImpl.hpp`

```cpp
class SocketTelemetrySourceImpl : public ITelemetrySource {
public:
    explicit SocketTelemetrySourceImpl(const std::string& host, int port);
    bool openSource() override;
    bool readSource(std::string& out) override;
};
```

---

### SomeIPTelemetryAdapter

Adapter for vsomeip-based remote telemetry.

**Header**: `src/sources/SomeIPTelemetryAdapter.hpp`

```cpp
class SomeIPTelemetryAdapter : public ITelemetrySource {
public:
    SomeIPTelemetryAdapter() = default;
    bool openSource() override;   // Initializes vsomeip client
    bool readSource(std::string& out) override;  // Requests load data
};
```

**Example**:
```cpp
SomeIPTelemetryAdapter source;
if (source.openSource()) {
    std::string loadData;
    if (source.readSource(loadData)) {
        float load = std::stof(loadData);
    }
}
```

---

## Sinks

### ConsoleSinkImpl

Thread-safe console output sink.

**Header**: `src/sinks/ConsoleSinkImpl.hpp`

```cpp
class ConsoleSinkImpl : public ILogSink {
public:
    void write(const LogMessage& msg) override;
};
```

**Thread Safety**: Uses static mutex for `std::cout`.

---

### FileSinkImpl

Thread-safe file output sink.

**Header**: `src/sinks/FileSinkImpl.hpp`

```cpp
class FileSinkImpl : public ILogSink {
public:
    explicit FileSinkImpl(const std::string& filepath);
    void write(const LogMessage& msg) override;
};
```

**Thread Safety**: Uses per-instance mutex.

---

## Concurrency

### ThreadPool

Worker thread pool for async task execution.

**Header**: `src/concurrency/ThreadPool.hpp`

```cpp
class ThreadPool {
public:
    explicit ThreadPool(std::size_t numThreads);
    ~ThreadPool();
    
    bool enqueue(std::function<void()> task);
};
```

| Method | Description |
|--------|-------------|
| `enqueue(task)` | Adds task to queue, returns false if shutdown |

**Thread Safety**: Fully thread-safe with mutex and condition variable.

---

### RingBuffer<T>

Thread-safe circular buffer.

**Header**: `src/concurrency/RingBuffer.hpp`

```cpp
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(std::size_t capacity);
    
    // Blocking operations
    void push(T&& value);       // Blocks if full
    T pop();                    // Blocks if empty
    
    // Non-blocking operations
    bool tryPush(T&& value);    // Returns false if full
    std::optional<T> tryPop();  // Returns nullopt if empty
    
    // Queries
    bool isEmpty() const noexcept;
    bool isFull() const noexcept;
    std::size_t count() const noexcept;
    std::size_t capacity() const noexcept;
};
```

**Example**:
```cpp
RingBuffer<LogMessage> buffer(100);

// Producer
buffer.tryPush(msg);  // Non-blocking

// Consumer  
auto result = buffer.tryPop();
if (result) {
    process(result.value());
}
```

---

## Utilities

### SafeFile

RAII wrapper for file descriptors.

**Header**: `src/utils/SafeFile.hpp`

```cpp
class SafeFile {
public:
    explicit SafeFile(const std::string& path, const char* mode = "r");
    ~SafeFile();  // Automatically closes file
    
    bool isOpen() const;
    FILE* get() const;
};
```

---

### SafeSocket

RAII wrapper for network sockets.

**Header**: `src/utils/SafeSocket.hpp`

```cpp
class SafeSocket {
public:
    SafeSocket();
    ~SafeSocket();  // Automatically closes socket
    
    bool connect(const std::string& host, int port);
    bool isConnected() const;
    int get() const;
};
```

---

## Types and Enums

### LogSinkType

```cpp
enum class LogSinkType {
    Console,
    File
};
```

### SeverityLvl

```cpp
enum class SeverityLvl {
    INFO,
    WARNING,
    CRITICAL
};
```

### TelemetrySrc

```cpp
enum class TelemetrySrc {
    CPU,
    GPU,
    RAM
};
```

### BuilderError

```cpp
enum class BuilderError {
    NO_SINKS_CONFIGURED,
    INVALID_BUFFER_SIZE,
    INVALID_THREADPOOL_SIZE,
    EMPTY_FILEPATH,
    NULL_SINK,
    SINK_CREATION_FAILED
};
```

### SinkCreationError

```cpp
enum class SinkCreationError {
    UNKNOWN_SINK_TYPE,
    EMPTY_FILEPATH,
    FILE_OPEN_FAILED
};
```

---

## Thread Safety Summary

| Class | Thread-Safe | Notes |
|-------|-------------|-------|
| `LogManager` | ✅ | Uses thread-safe buffer |
| `LogManagerBuilder` | ❌ | Build on single thread |
| `ThreadPool` | ✅ | Mutex + CV protected |
| `RingBuffer` | ✅ | Mutex + CV protected |
| `ConsoleSinkImpl` | ✅ | Static mutex |
| `FileSinkImpl` | ✅ | Per-instance mutex |
| `SomeIPTelemetrySourceImpl` | ✅ | Atomic + mutex |
| `FileTelemetrySourceImpl` | ❌ | Use one per thread |
