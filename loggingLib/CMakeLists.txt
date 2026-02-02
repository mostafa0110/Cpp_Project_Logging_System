add_library(logging
    src/core/LogManager.cpp
    src/core/LogManagerBuilder.cpp
    src/core/LogMessage.cpp
    src/core/LogSinkFactory.cpp
    src/sinks/ConsoleSinkImpl.cpp
    src/sinks/FileSinkImpl.cpp
    src/utils/SafeFile.cpp
    src/utils/SafeSocket.cpp
    src/sources/FileTelemetrySourceImpl.cpp
    src/sources/SocketTelemetrySourceImpl.cpp
)
target_link_libraries(logging PUBLIC magic_enum::magic_enum vsomeip3)
target_include_directories(logging 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
