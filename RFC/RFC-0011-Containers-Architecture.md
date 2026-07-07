# RFC-0011: Containers Architecture

Status: Draft accepted for NXM-0004
Version: 0.0.6

## Purpose

Nexiora requires containers that are fast, measurable, allocator-aware and portable. `NxVector` is the first container in the Nexiora Core Platform.

## Principles

1. Containers must not call the operating system directly.
2. Containers must use the Nexiora Memory Manager.
3. Containers must expose predictable ownership rules.
4. Containers must have unit tests and benchmarks.
5. Containers must prefer contiguous memory when performance matters.
6. Growth behavior must be explicit through policies.

## Initial container

`NxVector` is a dynamic contiguous array for fixed-size elements.

## Growth policies

- `NX_VECTOR_GROWTH_PERFORMANCE`: doubles capacity.
- `NX_VECTOR_GROWTH_BALANCED`: grows about 1.5x.
- `NX_VECTOR_GROWTH_MEMORY`: grows about 1.25x.

## Acceptance criteria

- Compiles on Windows with MSYS2/GCC and MSVC-oriented CMake presets.
- Passes unit tests.
- Benchmark must be added to `nexiora_bench` before promotion.
- Uses aligned memory through `nx_memory_allocate_aligned`.
