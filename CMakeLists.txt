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

add_subdirectory(loggingLib)
add_subdirectory(app)
