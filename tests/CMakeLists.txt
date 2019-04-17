cmake_minimum_required(VERSION 3.10)
project(cachew_tests)

find_program( clang_tidy NAMES clang-tidy PATHS /usr/local/opt/llvm/bin/)

message("\n#####################################################\n")
message("CMAKE_BUILD_TYPE:\t${CMAKE_BUILD_TYPE}")
message("CMAKE_INSTALL_PREFIX:\t${CMAKE_INSTALL_PREFIX}")
message("CMAKE_BINARY_DIR:\t${CMAKE_BINARY_DIR}")
message("CMAKE_MODULE_PATH:\t${CMAKE_MODULE_PATH}")
message("CMAKE_PREFIX_PATH:\t${CMAKE_PREFIX_PATH}")
message("CMAKE_SYSTEM_NAME:\t${CMAKE_SYSTEM_NAME}")
message("CMAKE_GENERATOR:\t${CMAKE_GENERATOR}")
message("CMAKE_C_COMPILER:\t${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER:\t${CMAKE_CXX_COMPILER}")
message("CLANG-TIDY:\t\t\t${clang_tidy}")
message("\n#####################################################\n")

add_executable(cachew_tests
        entry_point.cpp
        lru_cache.cpp
        lfu_cache.cpp)

target_link_libraries(cachew_tests
        cachew
        )

if(NOT WIN32)
    set_property(
            TARGET cachew_tests
            PROPERTY CXX_CLANG_TIDY ${clang_tidy}
    )
endif(NOT WIN32)
