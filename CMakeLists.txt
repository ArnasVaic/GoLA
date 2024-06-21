cmake_minimum_required(VERSION 3.10)

project(GoLC)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(libs/abseil-cpp)

# Add your source files here
add_executable(GoLC
        src/main.cpp
        src/game_of_life.hpp
        src/cycle.hpp
        src/frame.hpp
        src/transform.hpp
        src/frame.tpp
        src/process_rates_and_decays.hpp
        src/process_rates_and_decays.hpp
)

target_link_libraries(GoLC absl::base absl::numeric absl::hash)

# Test executable
# add_executable(FrameTests tests/frame_tests.cpp)
# add_test(NAME FrameTests COMMAND Tests)