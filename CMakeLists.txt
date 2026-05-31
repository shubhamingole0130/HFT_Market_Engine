cmake_minimum_required(VERSION 3.15)

project(HFT_Engine VERSION 1.0)

# 1. Enforce C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 1.1 Fix Windows Warning 
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# ==========================================
# 2. Find Dependencies (UPDATED FOR DOCKER)
# ==========================================

# Boost System
find_package(Boost REQUIRED COMPONENTS system)

# SpdLog: 
find_package(spdlog CONFIG)
if(NOT spdlog_FOUND)
    find_package(spdlog REQUIRED)
endif()

# GTest: 
find_package(GTest CONFIG)
if(NOT GTest_FOUND)
    find_package(GTest REQUIRED)
endif()

# ==========================================

# 3. Define the main Executable
add_executable(HFT_Engine src/main.cpp)

# Include directories
target_include_directories(HFT_Engine PRIVATE include)

# 4. Link the libraries to the executable
target_link_libraries(HFT_Engine
    PUBLIC
        Boost::system
        spdlog::spdlog
)

# 5. Define the Tests Executable
add_executable(HFT_Engine_Tests tests/test_main.cpp)

# Test includes
target_include_directories(HFT_Engine_Tests PRIVATE include)

# 6. Link libraries for the test executable
target_link_libraries(HFT_Engine_Tests
    PUBLIC
        GTest::gtest_main
        spdlog::spdlog
)