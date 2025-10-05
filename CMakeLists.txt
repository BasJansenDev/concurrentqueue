cmake_minimum_required(VERSION 3.14)
project(concurrentqueue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add gtest
add_subdirectory(extern/googletest)

# Your library (header-only, no sources yet)
add_library(concurrentqueue INTERFACE)
target_include_directories(concurrentqueue INTERFACE include)

# Your tests
add_executable(queue_tests tests/concurrentqueue-test.cpp)
target_link_libraries(queue_tests PRIVATE concurrentqueue gtest gtest_main pthread)

enable_testing()
include(GoogleTest)
gtest_discover_tests(queue_tests)

add_compile_options(-Wall -Wextra -Wpedantic)

