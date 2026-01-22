add_library(logging
    src/LogManager.cpp
    src/LogManagerBuilder.cpp
    src/LogMessage.cpp
    src/LogSinkFactory.cpp
    src/ConsoleSinkImpl.cpp
    src/FileSinkImpl.cpp
    src/SafeFile.cpp
    src/SafeSocket.cpp
    src/FileTelemetrySourceImpl.cpp
    src/SocketTelemetrySourceImpl.cpp
)
target_link_libraries(logging PUBLIC magic_enum::magic_enum)
target_include_directories(logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
