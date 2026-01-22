#pragma once

enum class LogSinkType {
    CONSOLE,
    FILE,
    SOCKET
};

enum class SeverityLvl {
    CRITICAL,
    WARNING,
    INFO
};

enum class TelemetrySrc {
    GPU,
    CPU,
    RAM
};

