# NRL-0007 — Autonomous Research Pipeline Orchestrator

Copy these files into the Nexiora repository root.

## Files

- Include/Nexiora/Research/NxAutonomousResearchPipeline.h
- Source/Research/NxAutonomousResearchPipeline.c
- Tests/Unit/NxAutonomousResearchPipelineTests.c
- Patches/CMakeLists.NRL-0007.patch
- BOOK updates

## CMake integration

Apply the CMake patch manually or add:

```cmake
Source/Research/NxAutonomousResearchPipeline.c
```

to `add_library(NexioraNCP STATIC ...)`.

Then add:

```cmake
add_executable(NxAutonomousResearchPipelineTests
    Tests/Unit/NxAutonomousResearchPipelineTests.c
)
target_link_libraries(NxAutonomousResearchPipelineTests PRIVATE NexioraNCP)
add_test(NAME NxAutonomousResearchPipelineTests COMMAND NxAutonomousResearchPipelineTests)
```

inside `if(NEXIORA_BUILD_TESTS)`.

## Verify

```powershell
cmake --build --preset release
ctest --test-dir .\Build\windows-msvc-release --output-on-failure
```

## Commit

```txt
NRL-0007: add autonomous research pipeline orchestrator
```
