cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(network-monitor)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/generators")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/generators")
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system unit_test_framework)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Filesystem REQUIRED)
find_package(nlohmann_json REQUIRED)

enable_testing()

set(LIB_SOURCES
    src/websocket-client.cpp
    src/file-downloader.cpp
)

add_library(network-monitor STATIC ${LIB_SOURCES})
target_compile_features(network-monitor
    PUBLIC
        cxx_std_14
        
)
target_link_libraries(network-monitor
    PUBLIC
        Boost::boost
        OpenSSL::SSL
        nlohmann_json::nlohmann_json
    PRIVATE
        CURL::libcurl
)
target_include_directories(network-monitor
    PUBLIC
        inc
)

set(TESTS_SOURCES
    tests/websocket-client.cpp
    tests/main.cpp
    tests/file-downloader.cpp
)

add_executable(network-monitor-tests ${TESTS_SOURCES})
target_compile_features(network-monitor-tests
    PRIVATE
        cxx_std_17
)
target_link_libraries(network-monitor-tests
    PRIVATE
        network-monitor
        Boost::boost
        Boost::unit_test_framework
        std::filesystem
)

target_compile_definitions(network-monitor-tests
    PRIVATE
        TESTS_CACERT_PEM="${CMAKE_CURRENT_SOURCE_DIR}/tests/cacert.pem" 
        TESTS_NETWORK_LAYOUT="${CMAKE_CURRENT_SOURCE_DIR}/tests/network-layout.json" 
)


add_test(
    NAME network-monitor-tests
    COMMAND $<TARGET_FILE:network-monitor-tests>   
)

set_tests_properties(network-monitor-tests PROPERTIES
    PASS_REGULAR_EXPRESSION ".*No errors detected"
)
