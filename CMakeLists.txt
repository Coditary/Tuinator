cmake_minimum_required(VERSION 3.16)
project(Tuinator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(TUINATOR_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(TUINATOR_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(TuinatorOptions)
include(TuinatorDependencies)
include(TuinatorTarget)

if(TUINATOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(TUINATOR_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
