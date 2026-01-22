cmake_minimum_required(VERSION 3.14)

project(CPP_Project_Logging_System LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(loggingLib)
add_subdirectory(libs/magic_enum)
add_subdirectory(app)
