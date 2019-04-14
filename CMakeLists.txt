cmake_minimum_required(VERSION 3.10)
project(cachew)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(cachew INTERFACE)
target_sources(cachew INTERFACE
        ${PROJECT_SOURCE_DIR}/include/cachew.hpp
        ${PROJECT_SOURCE_DIR}/include/cachew/lru_cache.hpp
        )

target_include_directories(cachew INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)


if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif()

add_subdirectory(tests)
