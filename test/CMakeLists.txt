# Host (PC) unit tests for the target independent parts of TaskMonitor.
#
# Uses the Unity test framework that ships with ESP-IDF, so IDF_PATH must be set (or pass -DUNITY_DIR=<path
# to Unity's src directory>). Requires a host C and C++ compiler (e.g. gcc/g++ from MinGW).
#
# Build and run:
#   cmake -S test -B test/build -G Ninja
#   cmake --build test/build
#   ctest --test-dir test/build --output-on-failure
cmake_minimum_required(VERSION 3.22)
project(TaskMonitorTests C CXX)

# Unity is C, the project code and tests are C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Lets VS Code (C/C++ extension) resolve includes in the test files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(UNITY_DIR "$ENV{IDF_PATH}/components/unity/unity/src" CACHE PATH "Path to the Unity test framework src directory")
if(NOT EXISTS "${UNITY_DIR}/unity.c")
    message(FATAL_ERROR "Unity not found at '${UNITY_DIR}'. Set IDF_PATH or pass -DUNITY_DIR=<path to Unity src>.")
endif()

add_library(unity STATIC ${UNITY_DIR}/unity.c)
target_include_directories(unity PUBLIC ${UNITY_DIR})

set(MAIN_DIR ${CMAKE_CURRENT_LIST_DIR}/../main)

# CheckinTracker is header only and has no target dependencies
add_executable(test_CheckinTracker
    test_CheckinTracker.cpp
)
target_include_directories(test_CheckinTracker PRIVATE ${MAIN_DIR}/Inc)
target_link_libraries(test_CheckinTracker PRIVATE unity)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_CheckinTracker PRIVATE -Wall -Wextra -Werror)
endif()

enable_testing()
add_test(NAME CheckinTracker COMMAND test_CheckinTracker)
