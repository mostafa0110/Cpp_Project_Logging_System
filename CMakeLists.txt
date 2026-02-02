cmake_minimum_required(VERSION 3.14)

project(CPP_Project_Logging_System LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Fetch magic_enum
include(FetchContent)
FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
    GIT_TAG v0.9.5
)
FetchContent_MakeAvailable(magic_enum)

# Find vsomeip
find_package(vsomeip3 REQUIRED)

add_subdirectory(loggingLib)
add_subdirectory(app)

# vsomeip configuration paths
set(VSOMEIP_CLIENT_CONFIG "${CMAKE_SOURCE_DIR}/config/vsomeip-client.json")
set(VSOMEIP_SERVER_CONFIG "${CMAKE_SOURCE_DIR}/config/vsomeip-server.json")

# Custom target to run main app with SomeIP client config
add_custom_target(run_app_someip
    COMMAND ${CMAKE_COMMAND} -E env "VSOMEIP_CONFIGURATION=${VSOMEIP_CLIENT_CONFIG}" $<TARGET_FILE:app>
    DEPENDS app
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running main app with SomeIP client configuration"
    VERBATIM
)

# Add test subdirectory
add_subdirectory(test)

