# SomeIP Test Server executable
add_executable(someip_test_server
    someip_test_main.cpp
)

target_link_libraries(someip_test_server
    PRIVATE vsomeip3
)

target_include_directories(someip_test_server
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

# SomeIP Test Client executable
add_executable(someip_test_client
    someip_test_client.cpp
)

target_link_libraries(someip_test_client
    PRIVATE logging
)

# Custom target to run SomeIP test server
add_custom_target(run_test_server
    COMMAND ${CMAKE_COMMAND} -E env "VSOMEIP_CONFIGURATION=${VSOMEIP_SERVER_CONFIG}" $<TARGET_FILE:someip_test_server>
    DEPENDS someip_test_server
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running SomeIP test server"
    VERBATIM
)

# Custom target to run SomeIP test client
add_custom_target(run_test_client
    COMMAND ${CMAKE_COMMAND} -E env "VSOMEIP_CONFIGURATION=${VSOMEIP_CLIENT_CONFIG}" $<TARGET_FILE:someip_test_client>
    DEPENDS someip_test_client
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running SomeIP test client"
    VERBATIM
)
