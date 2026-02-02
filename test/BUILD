# BUILD file for test executables

load("@rules_cc//cc:defs.bzl", "cc_binary")

# SomeIP test server
cc_binary(
    name = "someip_test_server",
    srcs = [
        "someip_test_main.cpp",
        "SomeIPTestServer.hpp",
    ],
    deps = [
        "@vsomeip//:vsomeip3",
    ],
    copts = ["-std=c++23"],
    env = {
        "VSOMEIP_CONFIGURATION": "config/vsomeip-server.json",
    },
)

# SomeIP test client
cc_binary(
    name = "someip_test_client",
    srcs = ["someip_test_client.cpp"],
    deps = [
        "//loggingLib:logging",
    ],
    copts = ["-std=c++23"],
    env = {
        "VSOMEIP_CONFIGURATION": "config/vsomeip-client.json",
    },
)
