cmake_minimum_required(VERSION 3.10)

project(GoLC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_library(golc INTERFACE)
target_include_directories(golc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/libs/eigen)
target_include_directories(golc INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_library(golc STATIC
#    src/game.hpp
#    src/cycle.hpp
#    src/frame.hpp
#    src/transform.hpp
#    src/frame.tpp
#    src/cycle.tpp
#    src/game.tpp)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE golc)

include(FetchContent)

# declare Catch2
FetchContent_Declare(
    Catch2                                                  # name of the content
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git   # the repository
    GIT_TAG        v2.13.7                                  # the tag
)

# make available
FetchContent_MakeAvailable(Catch2)

# testing binary
add_executable(golc_test
    tests/frame_tests.cpp
    tests/game_tests.cpp)

target_link_libraries(golc_test PRIVATE golc Catch2::Catch2)

# enable testing functionality
enable_testing()

# define tests
add_test(
    NAME tests
    COMMAND $<TARGET_FILE:golc_test> --success
)