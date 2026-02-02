# Architecture Documentation

This document explains the architectural decisions, design patterns, and component interactions in the C++ Logging System.

## Table of Contents

1. [Design Philosophy](#design-philosophy)
2. [Design Patterns](#design-patterns)
3. [Component Architecture](#component-architecture)
4. [Threading Model](#threading-model)
5. [Data Flow](#data-flow)

---

## Design Philosophy

The logging system is built around these core principles:

| Principle | Implementation |
|-----------|----------------|
| **Modularity** | Separate interfaces for sinks, sources, and formatters |
| **Thread Safety** | All shared components are mutex-protected |
| **Zero-Cost Abstractions** | Policy-based design for compile-time configuration |
| **Modern C++** | Uses C++23 features: `std::expected`, concepts, `std::optional` |
| **RAII** | All resources managed via RAII wrappers |

---

## Design Patterns

### 1. Builder Pattern (`LogManagerBuilder`)

**Problem**: LogManager has many configuration options (sinks, buffer size, thread pool size).

**Solution**: Fluent builder API with error accumulation.

```cpp
auto logger = LogManagerBuilder()
    .withConsoleSink()
    .withFileSink("app.log")
    .withBufferSize(100)
    .withthreadPoolSize(4)
    .tryBuild();  // Returns std::expected
```

**Why Builder over Constructor?**
- Optional parameters without constructor overloads
- Validation before object creation
- Readable, self-documenting configuration

### 2. Factory Pattern (`LogSinkFactory`)

**Problem**: Sink creation involves different parameters per type.

**Solution**: Centralized factory with type-safe creation.

```cpp
auto result = LogSinkFactory::createSink(LogSinkType::Console);
auto result = LogSinkFactory::createSink(LogSinkType::File, "path.log");
```

**Benefits**:
- Single point for sink instantiation
- Error handling via `std::expected`
- Easy to add new sink types

### 3. Strategy Pattern (`ILogSink`)

**Problem**: Different output destinations (console, file, network).

**Solution**: Abstract interface with concrete implementations.

```
ILogSink (interface)
    ├── ConsoleSinkImpl
    ├── FileSinkImpl
    └── [Future: NetworkSinkImpl]
```

**Benefits**:
- Runtime sink selection
- Multiple sinks simultaneously
- Easy testing with mock sinks

### 4. Policy-Based Design (`LogFormatter<Policy>`)

**Problem**: Different telemetry sources need different formatting rules.

**Solution**: Compile-time policies with static members.

```cpp
struct CpuPolicy {
    static constexpr TelemetrySrc context = TelemetrySrc::CPU;
    static constexpr float WARNING = 70.0f;
    static constexpr float CRITICAL = 90.0f;
    static constexpr std::string_view unit = "%";
    
    static SeverityLvl inferSeverity(float val) { ... }
};

LogFormatter<CpuPolicy> formatter;  // CPU-specific formatting
LogFormatter<RamPolicy> formatter;  // RAM-specific formatting
```

**Why Policy over Inheritance?**
- Zero runtime overhead
- No virtual function calls
- Compile-time optimization

### 5. Adapter Pattern (`SomeIPTelemetryAdapter`)

**Problem**: vsomeip has its own API, but we need `ITelemetrySource` interface.

**Solution**: Adapter wraps vsomeip implementation.

```
ITelemetrySource (interface)
    └── SomeIPTelemetryAdapter (adapter)
            └── SomeIPTelemetrySourceImpl (vsomeip-specific)
```

### 6. Singleton Pattern (`SomeIPTelemetrySourceImpl`)

**Problem**: vsomeip applications should only be instantiated once.

**Solution**: Meyer's Singleton for thread-safe lazy initialization.

```cpp
static SomeIPTelemetrySourceImpl& getInstance() {
    static SomeIPTelemetrySourceImpl instance;
    return instance;
}
```

### 7. RAII Wrappers (`SafeFile`, `SafeSocket`)

**Problem**: Raw file descriptors and sockets can leak.

**Solution**: RAII wrappers with automatic cleanup.

```cpp
SafeFile file("log.txt");  // Opens file
// ... use file ...
// Destructor automatically closes file
```

---

## Component Architecture

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Application Layer                              │
├─────────────────────────────────────────────────────────────────────────┤
│  TelemetrySources          LogFormatter           LogManagerBuilder     │
│  ├── FileTelemetrySource   └── Policy-based       └── Fluent builder    │
│  ├── SocketTelemetrySource     (CPU/RAM/GPU)                            │
│  └── SomeIPTelemetryAdapter                                             │
├─────────────────────────────────────────────────────────────────────────┤
│                              Core Layer                                  │
├─────────────────────────────────────────────────────────────────────────┤
│  LogManager                 LogMessage             LogSinkFactory       │
│  ├── RingBuffer (N)         └── Value type         └── Sink creation    │
│  ├── ThreadPool (M)                                                      │
│  └── Sinks[]                                                             │
├─────────────────────────────────────────────────────────────────────────┤
│                           Infrastructure Layer                           │
├─────────────────────────────────────────────────────────────────────────┤
│  ILogSink                   ITelemetrySource       RAII Wrappers        │
│  ├── ConsoleSinkImpl        └── Abstract interface ├── SafeFile         │
│  └── FileSinkImpl                                  └── SafeSocket       │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## Threading Model

### Thread Pool Architecture

```
Main Thread                     Thread Pool (N workers)
    │                               │
    │  log(msg)                     │
    │──────▶ RingBuffer ◀───────────│ worker threads pop messages
    │        (push)                 │
    │                               │
    │  flush()                      ▼
    │──────▶ Dispatch ──────▶ Sinks[i].write(msg)
```

### Synchronization Points

| Component | Mutex | Condition Variables | Purpose |
|-----------|-------|---------------------|---------|
| `ThreadPool` | `taskMutex` | `cv` | Task queue protection |
| `RingBuffer` | `bufferMutex` | `notEmpty`, `notFull` | Producer-consumer sync |
| `ConsoleSinkImpl` | Static `coutMutex` | - | Single `std::cout` |
| `FileSinkImpl` | Instance `fileMutex` | - | Per-file protection |

### Lost Wakeup Prevention

The ThreadPool destructor carefully prevents lost wakeups:

```cpp
~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(taskMutex);
        shutdown = true;  // Set under lock
    }
    cv.notify_all();  // Wake all threads
    for (auto& worker : workers) {
        worker.join();
    }
}
```

---

## Data Flow

### Telemetry to Log Flow

```
1. TelemetrySource.readSource(raw)    // Read raw data
        │
        ▼
2. LogFormatter.formatDataToLogMsg()  // Parse + create LogMessage
        │
        ▼
3. LogManager.log(msg)                // Push to RingBuffer
        │
        ▼
4. LogManager.flush()                 // Pop all and dispatch
        │
        ▼
5. ThreadPool.enqueue(task)           // Submit to workers
        │
        ▼
6. ILogSink.write(msg)                // Execute on worker thread
```

### SomeIP Request-Response Flow

```
Client                                   Server
   │                                        │
   │  request_service(0x1234)              │
   │───────────────────────────────────────▶│
   │                                        │ offer_service()
   │                                        │
   │  ON_AVAILABLE callback                 │
   │◀───────────────────────────────────────│
   │                                        │
   │  send(request)                         │
   │───────────────────────────────────────▶│
   │                                        │ onMessage()
   │                                        │ create_response()
   │  onMessage() callback                  │
   │◀───────────────────────────────────────│ send(response)
   │                                        │
```

---

## Error Handling Strategy

The system uses **monadic error handling** instead of exceptions:

| Return Type | Usage |
|-------------|-------|
| `std::expected<T, E>` | Operations that can fail with specific errors |
| `std::optional<T>` | Operations that may not produce a value |
| `bool` | Simple success/failure |

Example:
```cpp
// Factory returns expected
std::expected<std::shared_ptr<ILogSink>, SinkCreationError> result = 
    LogSinkFactory::createSink(type);

// Builder returns expected
std::expected<std::unique_ptr<LogManager>, BuilderError> result = 
    builder.tryBuild();

// Formatter returns optional
std::optional<LogMessage> msg = formatter.formatDataToLogMsg(raw);
```

---

## Future Extensibility

The architecture supports easy extension:

| Extension | How to Add |
|-----------|------------|
| New Sink | Implement `ILogSink`, add to factory |
| New Telemetry Source | Implement `ITelemetrySource` |
| New Log Policy | Create new policy struct |
| Network Logging | Implement `NetworkSinkImpl` with `SafeSocket` |
