cmake_minimum_required(VERSION 4.0)
project(network-monitor)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_executable(network-monitor src/main.cpp)
target_compile_features(network-monitor
        PRIVATE
        cxx_std_11
)
