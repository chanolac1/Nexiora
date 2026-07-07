cmake_minimum_required(VERSION 3.25)

project(Nexiora
    VERSION 0.0.4
    DESCRIPTION "Nexiora Core Platform - HAL"
    LANGUAGES C)

# Nexiora targets C23. MSVC still exposes the most recent C support through
# /std:clatest on many Visual Studio 2022 installations, so we request C17
# from CMake for MSVC and add /std:clatest explicitly. The current code avoids
# compiler-specific C23-only syntax so Genesis remains buildable on Windows.
if(MSVC)
    set(CMAKE_C_STANDARD 17)
else()
    set(CMAKE_C_STANDARD 23)
endif()
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(NEXIORA_BUILD_TESTS "Build Nexiora tests" ON)
option(NEXIORA_ENABLE_BENCHMARKS "Build Nexiora benchmarks" ON)

if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8 /std:clatest)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_library(NexioraNCP STATIC
    Source/NCP/Common/NxResult.c
    Source/NCP/Memory/NxMemory.c
    Source/NCP/Logging/NxLog.c
    Source/NCP/Hardware/NxHardware.c
    Source/NCP/Benchmark/NxBenchmark.c
    Source/NCP/Evidence/NxEvidence.c
    Source/NCP/Runtime/NxRuntime.c
)

target_include_directories(NexioraNCP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(NexioraNCP PRIVATE -mxsave)
endif()

target_compile_definitions(NexioraNCP PUBLIC
    NEXIORA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    NEXIORA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    NEXIORA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

add_executable(nexiora Source/NxBootstrap.c)
target_link_libraries(nexiora PRIVATE NexioraNCP)

if(NEXIORA_BUILD_TESTS)
    enable_testing()
    add_executable(nexiora_tests Tests/NxCoreTests.c)
    target_link_libraries(nexiora_tests PRIVATE NexioraNCP)
    add_test(NAME NexioraCoreTests COMMAND nexiora_tests)
endif()

if(NEXIORA_ENABLE_BENCHMARKS)
    add_executable(nexiora_bench Tests/NxBenchmarkMain.c)
    target_link_libraries(nexiora_bench PRIVATE NexioraNCP)
endif()
