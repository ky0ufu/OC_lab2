cmake_minimum_required(VERSION 3.5)
project(bgproc CXX)
set(CMAKE_CXX_STANDARD 11)


add_library(bgproc STATIC bgproc.cpp)
add_executable(test_app test.cpp)
target_link_libraries(test_app bgproc)

