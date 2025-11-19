cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(network-monitor)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system unit_test_framework)

enable_testing()

set(LIB_SOURCES
    src/websocket-client.cpp
)

add_library(network-monitor STATIC ${LIB_SOURCES})
target_compile_features(network-monitor
    PUBLIC
        cxx_std_14
)
target_link_libraries(network-monitor
    PUBLIC
        Boost::boost
)
target_include_directories(network-monitor
    PUBLIC
        inc
)

set(TESTS_SOURCES
    tests/websocket-client.cpp
    tests/main.cpp
)

add_executable(network-monitor-tests ${TESTS_SOURCES})
target_compile_features(network-monitor-tests
    PRIVATE
        cxx_std_14
)
target_link_libraries(network-monitor-tests
    PRIVATE
        network-monitor
        Boost::boost
        Boost::unit_test_framework
)

add_test(
    NAME network-monitor-tests
    COMMAND $<TARGET_FILE:network-monitor-tests>   
)

set_tests_properties(network-monitor-tests PROPERTIES
    PASS_REGULAR_EXPRESSION ".*No errors detected"
)
