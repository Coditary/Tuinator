cmake_minimum_required(VERSION 3.16)
project(tuinator_find_package_smoke LANGUAGES CXX)

find_package(Tuinator CONFIG REQUIRED)

add_executable(find_package_smoke main.cpp)
target_link_libraries(find_package_smoke PRIVATE tuinator::tuinator)
