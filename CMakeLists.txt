cmake_minimum_required(VERSION 3.14)
project(fast_isqrt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Compiler Info Output
# -----------------------------------------------------------------------------
message(STATUS "==================================================")
message(STATUS " [Compiler Configuration Summary]")
message(STATUS "   - Compiler ID       : ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "   - Compiler Version  : ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "   - Compiler Path     : ${CMAKE_CXX_COMPILER}")
message(STATUS "   - C++ Standard      : C++${CMAKE_CXX_STANDARD}")
message(STATUS "   - Target Platform   : ${CMAKE_SYSTEM_NAME} (${CMAKE_SYSTEM_PROCESSOR})")
message(STATUS "==================================================")

# 1. Header-only ライブラリの定義
add_library(fast_isqrt INTERFACE)
target_include_directories(fast_isqrt INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

option(BUILD_TESTS "Build test executable" ON)
option(BUILD_BENCHMARKS "Build benchmark executable" OFF)

# 2. テストのビルド設定
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_fast_isqrt tests/test_fast_isqrt.cpp)
    target_link_libraries(test_fast_isqrt PRIVATE fast_isqrt)
    
    # GCC/Clang 向けフラグ (UCRT64 含む)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(test_fast_isqrt PRIVATE -Wall -Wextra -O2)
    endif()

    add_test(NAME test_fast_isqrt COMMAND test_fast_isqrt)
    set_tests_properties(test_fast_isqrt PROPERTIES FORCE_OUTPUT_ON_FAILURE ON)
endif()

# 3. ベンチマークのビルド設定
if(BUILD_BENCHMARKS)
    add_executable(bench_fast_isqrt benchmarks/bench.cpp)
    target_link_libraries(bench_fast_isqrt PRIVATE fast_isqrt)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(bench_fast_isqrt PRIVATE -O3 -march=native)
    elseif(MSVC)
        target_compile_options(bench_fast_isqrt PRIVATE /O2 /Oi /Ot)
    endif()
endif()