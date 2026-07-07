# NXM-0001: Memory Manager

This document describes the first formal Nexiora Core Platform module.

## Files

```text
Include/Nexiora/NCP/Memory/NxMemory.h
Source/NCP/Memory/NxMemory.c
Tests/NxCoreTests.c
Tests/NxBenchmarkMain.c
RFC/RFC-0003-Memory-Manager.md
```

## Main Features

- 64-byte default aligned allocation.
- Explicit aligned allocation.
- Zeroed allocation.
- Allocation statistics.
- Arena allocator for temporary memory.
- Benchmark integration.

## Build

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
```

## Benchmark

```powershell
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 100000
```

Expected benchmark names:

```text
nx_memory_allocate/free 64 bytes
nx_memory_allocate_aligned/free 64 bytes
nx_arena_allocate/reset 64 bytes
```

## Notes

The arena allocator should normally be faster than general allocation because it only advances an offset and resets in bulk. It does not free individual blocks.
