# RFC-0003: Nexiora Memory Manager

Version: 0.0.3  
Status: Accepted for Genesis development  
Module: NXM-0001

## Purpose

The Nexiora Memory Manager centralizes allocation, alignment, measurement and future memory auditing for the Nexiora Core Platform.

## Design Goals

1. Avoid direct `malloc` usage in Nexiora business logic.
2. Provide 64-byte default alignment for SIMD-friendly code paths.
3. Track allocation counts, free counts, current bytes, peak bytes and failed allocations.
4. Provide an arena allocator for high-frequency temporary allocations.
5. Keep the implementation small and dependency-free for Genesis.

## Public API

Public declarations live in:

```text
Include/Nexiora/NCP/Memory/NxMemory.h
```

Implementation lives in:

```text
Source/NCP/Memory/NxMemory.c
```

## Allocation Types

### General Allocation

`nx_memory_allocate(size)` returns a 64-byte aligned block. It is appropriate for long-lived objects and API-level allocations.

### Explicit Aligned Allocation

`nx_memory_allocate_aligned(size, alignment)` supports explicit power-of-two alignment.

### Arena Allocation

`NxArena` is intended for temporary high-volume allocations where individual frees are unnecessary. It is reset in bulk using `nx_arena_reset()`.

## Acceptance Criteria

The module is accepted when:

1. The full project builds on Windows with MSVC/CMake.
2. Core tests pass.
3. Memory statistics return to zero after tests.
4. Allocated blocks satisfy requested alignment.
5. Benchmarks produce historical evidence entries.

## Future Work

- Leak report with allocation source metadata.
- Thread-local arenas.
- Pool allocator for fixed-size blocks.
- Guard pages in debug mode.
- Canary checks for buffer overrun detection.
- Hardware-specialized allocator policies.
