cmake_minimum_required(VERSION 3.25)

project(Nexiora
    VERSION 0.0.11
    DESCRIPTION "Nexiora Research Lab - Manifest and Registry Engine"
    LANGUAGES C
)

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
    Source/NCP/Runtime/NxRuntime.c
    Source/NCP/Memory/NxMemory.c
    Source/NCP/Logging/NxLog.c
    Source/NCP/Hardware/NxHardware.c
    Source/NCP/String/NxString.c
    Source/NCP/Containers/NxVector.c
    Source/NCP/Benchmark/NxBenchmark.c
    Source/NCP/Evidence/NxEvidence.c
    Source/NCP/Research/NxJournal.c
    Source/NCP/Research/NxExperimentRunner.c
    Source/NCP/Research/NxResearchEvidence.c
    Source/Research/NxExperiment.c
    Source/Research/NxResearchKernel.c
    Source/Research/NxManifest.c
    Source/Research/NxRegistry.c
)

target_include_directories(NexioraNCP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

target_compile_definitions(NexioraNCP PUBLIC
    NEXIORA_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    NEXIORA_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    NEXIORA_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(NexioraNCP PRIVATE -mxsave)
endif()

add_executable(nexiora Source/NxBootstrap.c)
target_link_libraries(nexiora PRIVATE NexioraNCP)

if(NEXIORA_ENABLE_BENCHMARKS)
    add_executable(nexiora_bench Tests/NxBenchmarkMain.c)
    target_link_libraries(nexiora_bench PRIVATE NexioraNCP)
endif()

if(NEXIORA_BUILD_TESTS)
    enable_testing()

    add_executable(nexiora_tests Tests/NxCoreTests.c)
    target_link_libraries(nexiora_tests PRIVATE NexioraNCP)
    add_test(NAME NexioraCoreTests COMMAND nexiora_tests)

    add_executable(NxVectorTests Tests/Unit/NxVectorTests.c)
    target_link_libraries(NxVectorTests PRIVATE NexioraNCP)
    add_test(NAME NxVectorTests COMMAND NxVectorTests)

    add_executable(NxResearchTests Tests/Unit/NxResearchTests.c)
    target_link_libraries(NxResearchTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchTests COMMAND NxResearchTests)

    add_executable(NxJournalTests
        Tests/Unit/NxJournalTests.c
    )
    target_link_libraries(NxJournalTests PRIVATE NexioraNCP)
    add_test(NAME NxJournalTests COMMAND NxJournalTests)

    add_executable(NxExperimentRunnerTests
        Tests/Unit/NxExperimentRunnerTests.c
    )
    target_link_libraries(NxExperimentRunnerTests PRIVATE NexioraNCP)
    add_test(NAME NxExperimentRunnerTests COMMAND NxExperimentRunnerTests)
    
    add_executable(NxResearchEvidenceTests
    Tests/Unit/NxResearchEvidenceTests.c
    )
    
    target_link_libraries(NxResearchEvidenceTests PRIVATE NexioraNCP)
    add_test(NAME NxResearchEvidenceTests COMMAND NxResearchEvidenceTests)

endif()
