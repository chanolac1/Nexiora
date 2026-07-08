# RFC-0014: Nexiora Developer Platform

Status: Accepted
Version: 0.0.7

## Summary

Nexiora requires a reproducible developer platform before the project grows further.
This RFC establishes `nxbuild` as the official workflow frontend.

## Goals

- Reduce manual integration errors.
- Standardize build, test, benchmark, approval, and diagnostics.
- Support module-level benchmarks.
- Prepare the transition from PowerShell script to native `nxbuild.exe`.

## Non-Goals

- Replacing CMake in this version.
- Implementing the final native build tool in this version.

## Accepted Commands

- `doctor`
- `configure`
- `build`
- `test`
- `benchmark`
- `approve`
- `clean`
- `all`

## Future Work

`nxbuild.ps1` will be replaced by a native C application that controls CMake, tests,
benchmarks, packaging, and release evidence.
