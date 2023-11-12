cmake_minimum_required(VERSION 3.10)

project(GoLC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add your source files here
add_executable(GoLC main.cpp board.h)