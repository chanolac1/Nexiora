# RFC-0004: Hardware Abstraction Layer

Version: 0.0.4 HAL  
Status: Accepted  
Module: NXM-0002

## Purpose

The Hardware Abstraction Layer (HAL) gives Nexiora a reliable, measurable description of the machine it is running on. Future components such as SIMD strings, tensor kernels, allocators, schedulers, and model inference engines must select implementations from evidence, not assumptions.

## Responsibilities

- Detect platform and CPU vendor.
- Detect logical processors and baseline physical core estimate.
- Detect total and available memory.
- Detect cache line and cache-size metadata when available.
- Detect CPU instruction capabilities such as SSE, AVX, AVX2, AVX-512, FMA, AES, BMI, and POPCNT.
- Expose the information through a stable C API.

## Non-goals

- GPU detection is not part of this version.
- NUMA topology is not part of this version.
- Per-core frequency telemetry is not part of this version.

## Acceptance Criteria

- Nexiora builds on Windows with MSVC and CMake.
- `nexiora.exe` prints HAL data during runtime status.
- Tests validate that HAL returns a valid platform, processor count, and cache-line value.
- `nexiora_bench.exe` includes a benchmark for `nx_hardware_query`.

## Design Principle

HAL is a foundation for adaptive runtime dispatch. A future function such as `nx_memcpy` or `nx_matmul` must be able to ask HAL which implementation is safe and preferred on the current hardware.
