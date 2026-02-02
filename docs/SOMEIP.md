# SomeIP Telemetry Integration

This document covers the vsomeip integration for remote telemetry collection.

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [Configuration](#configuration)
4. [Message Protocol](#message-protocol)
5. [Testing](#testing)
6. [Troubleshooting](#troubleshooting)

---

## Overview

The SomeIP integration allows collecting telemetry from remote services using the SOME/IP (Scalable service-Oriented MiddlewarE over IP) protocol, commonly used in automotive systems.

### Key Components

| Component | File | Purpose |
|-----------|------|---------|
| `SomeIPTelemetryAdapter` | `sources/SomeIPTelemetryAdapter.hpp` | ITelemetrySource adapter |
| `SomeIPTelemetrySourceImpl` | `sources/SomeIPTelemetrySourceImpl.hpp` | vsomeip client implementation |
| `SomeIPTestServer` | `test/SomeIPTestServer.hpp` | Mock server for testing |

---

## Architecture

### Class Hierarchy

```
ITelemetrySource (interface)
    └── SomeIPTelemetryAdapter
            └── SomeIPTelemetrySourceImpl (singleton)
                    └── vsomeip::application
```

### Client-Server Model

```
┌──────────────────────┐         ┌──────────────────────┐
│  TelemetryClient     │         │  TelemetryServer     │
│  (Application)       │         │  (Test Server)       │
├──────────────────────┤         ├──────────────────────┤
│  App ID: 0x1111      │◀───────▶│  App ID: 0x2222      │
│  Requests Service    │  SOME/IP│  Offers Service      │
│  0x1234:0x5678       │         │  0x1234:0x5678       │
└──────────────────────┘         └──────────────────────┘
```

---

## Configuration

### Service Identifiers

| Identifier | Value | Description |
|------------|-------|-------------|
| SERVICE_ID | `0x1234` | Telemetry service identifier |
| INSTANCE_ID | `0x5678` | Service instance |
| METHOD_ID | `0x0001` | Get load percentage method |
| MAJOR_VERSION | `1` | Service major version |
| MINOR_VERSION | `0` | Service minor version |

### Client Configuration (`config/vsomeip-client.json`)

```json
{
    "unicast": "127.0.0.1",
    "logging": {
        "level": "info",
        "console": true
    },
    "applications": [{
        "name": "TelemetryClient",
        "id": "0x1111"
    }],
    "clients": [{
        "service": "0x1234",
        "instance": "0x5678",
        "reliable": [30509]
    }],
    "service-discovery": {
        "enable": true,
        "multicast": "224.224.224.245",
        "port": 30490,
        "protocol": "udp"
    }
}
```

### Server Configuration (`config/vsomeip-server.json`)

```json
{
    "unicast": "127.0.0.1",
    "applications": [{
        "name": "TelemetryServer",
        "id": "0x2222"
    }],
    "services": [{
        "service": "0x1234",
        "instance": "0x5678",
        "reliable": { "port": 30509 }
    }],
    "routing": "TelemetryServer",
    "service-discovery": {
        "enable": true,
        "multicast": "224.224.224.245",
        "port": 30490
    }
}
```

### Key Configuration Notes

| Field | Description |
|-------|-------------|
| `routing` | Only ONE application can be routing manager (typically server) |
| `reliable` | TCP port for reliable communication |
| `service-discovery` | SD multicast settings for service announcement |

---

## Message Protocol

### Request (Client → Server)

```
┌─────────────┬─────────────┬─────────────┐
│ Service ID  │ Instance ID │ Method ID   │
│ 0x1234      │ 0x5678      │ 0x0001      │
└─────────────┴─────────────┴─────────────┘
│        No payload (empty request)       │
└─────────────────────────────────────────┘
```

### Response (Server → Client)

```
┌─────────────┬─────────────┬─────────────┐
│ Service ID  │ Instance ID │ Method ID   │
│ 0x1234      │ 0x5678      │ 0x0001      │
└─────────────┴─────────────┴─────────────┘
│              Payload (4 bytes)          │
│         float: load percentage          │
└─────────────────────────────────────────┘
```

### Serialization

```cpp
// Server: Serialize float to payload
float loadValue = 75.5f;
std::vector<vsomeip::byte_t> data(sizeof(float));
std::memcpy(data.data(), &loadValue, sizeof(float));

// Client: Deserialize payload to float
float loadValue;
std::memcpy(&loadValue, payload->get_data(), sizeof(float));
```

---

## Testing

### Step 1: Start the Test Server

```bash
cd build
make run_test_server
```

Expected output:
```
=== SomeIP Telemetry Test Server ===
Service ID: 0x1234
Instance ID: 0x5678
Method ID: 0x0001

Starting server...
Server is running. Press Ctrl+C to stop.
Responding with fixed load value: 75.5%
```

### Step 2: Run Test Client

In a separate terminal:
```bash
cd build
make run_test_client
```

Expected output:
```
=== SomeIP Telemetry Test Client ===
Client initialized. Waiting for server...

[Request #1] Received load: 75.5%
[Request #2] Received load: 75.5%
...
```

### Step 3: Run Main Application

```bash
cd build
make run_app_someip
```

This runs the full application with local Linux telemetry plus SomeIP.

---

## Troubleshooting

### Error: "configured as routing but other routing manager present"

**Cause**: Multiple applications trying to be routing manager.

**Solution**: Only the server config should have `"routing": "TelemetryServer"`. Client config should NOT have a routing field.

### Error: "Failed to initialize SomeIP client"

**Cause**: vsomeip application init failed.

**Possible causes**:
1. No routing manager running (start server first)
2. Configuration file not found
3. Port already in use

**Solution**:
```bash
# Check if server is running
ps aux | grep someip

# Verify config path
echo $VSOMEIP_CONFIGURATION
```

### Client shows "Waiting for server to become available..."

**Cause**: Service discovery hasn't found the server yet.

**Solutions**:
1. Ensure server is running
2. Check multicast settings match
3. Verify SERVICE_ID and INSTANCE_ID match

### vsomeip logs show "[error] Security disabled!"

**Note**: This is informational, not an error. Security is intentionally disabled for local testing.

---

## API Usage

### Basic Client Usage

```cpp
#include "sources/SomeIPTelemetryAdapter.hpp"

SomeIPTelemetryAdapter source;

// Initialize (non-blocking)
if (!source.openSource()) {
    // Server not available, handle gracefully
}

// Poll for data
std::string loadData;
if (source.readSource(loadData)) {
    float load = std::stof(loadData);
    std::cout << "Remote load: " << load << "%" << std::endl;
}
```

### Configuration via Environment

```bash
# Set config file
export VSOMEIP_CONFIGURATION=/path/to/config.json

# Or use CMake targets (automatically set)
make run_test_client
make run_test_server
make run_app_someip
```

---

## Sequence Diagrams

### Service Discovery

```
TelemetryClient                  SD Daemon                  TelemetryServer
      │                              │                              │
      │                              │◀──── offer_service() ────────│
      │                              │                              │
      │── request_service() ────────▶│                              │
      │                              │                              │
      │◀── ON_AVAILABLE(1234.5678) ──│                              │
      │                              │                              │
```

### Request-Response

```
TelemetryClient                                    TelemetryServer
      │                                                  │
      │  create_request(service, instance, method)       │
      │                                                  │
      │── send(request) ────────────────────────────────▶│
      │                                                  │ onMessage()
      │                                                  │ generate load value
      │                                                  │ create_response()
      │◀──────────────────────────────── send(response) ─│
      │  onMessage()                                     │
      │  parse payload                                   │
      │                                                  │
```
