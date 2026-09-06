cmake_minimum_required(VERSION 3.16)
project(tuinator_fetchcontent_smoke LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
    tuinator
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../../..
)
set(TUINATOR_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(TUINATOR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(TUINATOR_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(tuinator)

add_executable(fetchcontent_smoke main.cpp)
target_link_libraries(fetchcontent_smoke PRIVATE tuinator::tuinator)
