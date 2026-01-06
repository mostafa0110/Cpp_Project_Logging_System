add_library(logging
    src/LogManager.cpp
    src/LogMessage.cpp
    src/ConsoleSinkImpl.cpp
    src/FileSinkImpl.cpp
)

target_include_directories(logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc )
