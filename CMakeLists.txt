cmake_minimum_required(VERSION 3.14)
project(trading_benchmark VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif()

# Math definitions for cross-platform compatibility
add_compile_definitions(_USE_MATH_DEFINES)

# Options
option(BUILD_TESTS "Build unit tests" ON)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Libraries
add_library(position lib/position.cc lib/position.h)
target_include_directories(position PUBLIC ${CMAKE_SOURCE_DIR})

add_library(benchmark lib/benchmark.cc lib/benchmark.h)
target_include_directories(benchmark PUBLIC ${CMAKE_SOURCE_DIR})

add_library(greeks lib/greeks.cc lib/greeks.h)
target_include_directories(greeks PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(greeks PUBLIC position)

add_library(monte_carlo lib/monte_carlo.cc lib/monte_carlo.h)
target_include_directories(monte_carlo PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(monte_carlo PUBLIC position)

add_library(aggregator lib/aggregator.cc lib/aggregator.h)
target_include_directories(aggregator PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(aggregator PUBLIC position)

# System library (CPU affinity, NUMA, etc.)
add_library(system_lib lib/system.cc lib/system.h)
target_include_directories(system_lib PUBLIC ${CMAKE_SOURCE_DIR})

# Link NUMA library on Linux
if(UNIX AND NOT APPLE)
    find_library(NUMA_LIBRARY numa)
    if(NUMA_LIBRARY)
        target_link_libraries(system_lib PUBLIC ${NUMA_LIBRARY})
        target_compile_definitions(system_lib PRIVATE HAVE_NUMA)
    else()
        message(WARNING "libnuma not found. NUMA support will be disabled.")
    endif()
endif()

# Main benchmark executable
add_executable(risk_benchmark apps/risk_benchmark.cc)
target_link_libraries(risk_benchmark PRIVATE
    position
    benchmark
    greeks
    monte_carlo
    aggregator
    system_lib
)

# Calculator example
add_library(math lib/math.cc lib/math.h)
target_include_directories(math PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(calculator apps/calculator.cc)
target_link_libraries(calculator PRIVATE math)

# Installation
install(TARGETS risk_benchmark calculator
    RUNTIME DESTINATION bin
)

install(FILES
    README.md
    BENCHMARK_RESULTS.md
    DESTINATION share/doc/trading_benchmark
)

# Tests
if(BUILD_TESTS)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(greeks_test lib/greeks_test.cc)
    target_link_libraries(greeks_test PRIVATE greeks position GTest::gtest_main)

    add_executable(monte_carlo_test lib/monte_carlo_test.cc)
    target_link_libraries(monte_carlo_test PRIVATE monte_carlo position GTest::gtest_main)

    add_executable(aggregator_test lib/aggregator_test.cc)
    target_link_libraries(aggregator_test PRIVATE aggregator position GTest::gtest_main)

    add_executable(math_test lib/math_test.cc)
    target_link_libraries(math_test PRIVATE math GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(greeks_test)
    gtest_discover_tests(monte_carlo_test)
    gtest_discover_tests(aggregator_test)
    gtest_discover_tests(math_test)
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "trading-benchmark")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Trading System CPU Benchmark Suite")
set(CPACK_PACKAGE_VENDOR "Performance Benchmark")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

# Platform-specific package generators
if(APPLE)
    set(CPACK_GENERATOR "TGZ;ZIP")
elseif(UNIX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
else()
    set(CPACK_GENERATOR "ZIP")
endif()

# DEB specific
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Maintainer")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")

# RPM specific
set(CPACK_RPM_PACKAGE_LICENSE "MIT")

include(CPack)
