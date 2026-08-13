cmake_minimum_required(VERSION 3.15)

project(HFT_Engine VERSION 1.0)

# ==========================================
# 1. Enforce C++20 Standard
# ==========================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==========================================
# 1.1 Windows Configuration
# ==========================================

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

# ==========================================
# 2. Find Dependencies
# ==========================================

# Boost 1.90
# Boost.System is header-only in newer Boost versions,
# so we do not request the separate "system" component.
find_package(Boost REQUIRED)

# SpdLog
find_package(spdlog CONFIG)

if(NOT spdlog_FOUND)
    find_package(spdlog REQUIRED)
endif()

# GTest
find_package(GTest CONFIG)

if(NOT GTest_FOUND)
    find_package(GTest REQUIRED)
endif()

# ==========================================
# 3. Main HFT Engine Executable
# ==========================================

add_executable(HFT_Engine
    src/main.cpp
)

# Include directories
target_include_directories(HFT_Engine
    PRIVATE
        include
)

# ==========================================
# 4. Link Libraries
# ==========================================

target_link_libraries(HFT_Engine
    PUBLIC
        spdlog::spdlog
)

# Use header-only Boost.System
target_compile_definitions(HFT_Engine
    PRIVATE
        BOOST_ERROR_CODE_HEADER_ONLY
)

# ==========================================
# 5. Tests Executable
# ==========================================

add_executable(HFT_Engine_Tests test/test_main.cpp)

# Test include directories
target_include_directories(HFT_Engine_Tests
    PRIVATE
        include
)

# ==========================================
# 6. Link Test Libraries
# ==========================================

target_link_libraries(HFT_Engine_Tests
    PUBLIC
        GTest::gtest_main
        spdlog::spdlog
)

# Use header-only Boost.System for tests as well
target_compile_definitions(HFT_Engine_Tests
    PRIVATE
        BOOST_ERROR_CODE_HEADER_ONLY
)

# ==========================================
# 7. Enable Testing
# ==========================================

enable_testing()

include(GoogleTest)

gtest_discover_tests(HFT_Engine_Tests)