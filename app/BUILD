# BUILD file for main application

load("@rules_cc//cc:defs.bzl", "cc_binary")

cc_binary(
    name = "app",
    srcs = ["src/main.cpp"],
    deps = [
        "//loggingLib:logging",
    ],
    copts = ["-std=c++23"],
    env = {
        "VSOMEIP_CONFIGURATION": "config/vsomeip-client.json",
    },
)
