cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(network-monitor)

if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conaninfo.txt)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}")
endif()
find_package(Boost 1.74 REQUIRED COMPONENTS system)
add_executable(network-monitor src/main.cpp)
target_compile_features(network-monitor
        PRIVATE
        cxx_std_11
)
target_link_libraries(network-monitor
    PRIVATE
        Boost::boost
)
