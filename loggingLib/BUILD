# BUILD file for loggingLib
# Defines the logging library with vsomeip support

load("@rules_cc//cc:defs.bzl", "cc_library")

# Full logging library with SomeIP support
cc_library(
    name = "logging",
    srcs = [
        "src/core/LogManager.cpp",
        "src/core/LogManagerBuilder.cpp",
        "src/core/LogMessage.cpp",
        "src/core/LogSinkFactory.cpp",
        "src/sinks/ConsoleSinkImpl.cpp",
        "src/sinks/FileSinkImpl.cpp",
        "src/utils/SafeFile.cpp",
        "src/utils/SafeSocket.cpp",
        "src/sources/FileTelemetrySourceImpl.cpp",
        "src/sources/SocketTelemetrySourceImpl.cpp",
    ],
    hdrs = glob([
        "include/**/*.hpp",
        "src/**/*.hpp",
    ]),
    includes = [
        "include",
        "src",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@magic_enum",
        "@vsomeip//:vsomeip3",
    ],
    copts = ["-std=c++23"],
)
