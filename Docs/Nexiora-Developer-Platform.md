# Nexiora Developer Platform

Version: 0.0.7
Codename: Developer Platform

## Purpose

Nexiora Developer Platform formalizes the local development workflow.
The goal is to stop integrating modules manually and to make build, test,
benchmark, approval, and environment diagnostics reproducible.

## nxbuild.ps1

`Scripts/nxbuild.ps1` is the temporary PowerShell frontend for Nexiora build orchestration.
It will eventually be replaced by `nxbuild.exe` written in C.

Supported commands:

```powershell
.\Scripts\nxbuild.ps1 doctor
.\Scripts\nxbuild.ps1 configure
.\Scripts\nxbuild.ps1 build
.\Scripts\nxbuild.ps1 test
.\Scripts\nxbuild.ps1 benchmark containers
.\Scripts\nxbuild.ps1 benchmark --module containers
.\Scripts\nxbuild.ps1 approve
.\Scripts\nxbuild.ps1 clean
.\Scripts\nxbuild.ps1 all containers
```

## Module Benchmarks

The benchmark runner supports module-level evidence:

- `all`
- `memory`
- `hardware` or `hal`
- `string` or `strings`
- `containers` or `vector`

This prevents unrelated benchmark noise from blocking the promotion of a module that did not change.

## PowerShell Security Warning

If Windows blocks the script because it came from the Internet, run:

```powershell
Unblock-File .\Scripts\nxbuild.ps1
```

Then execute the desired command again.

## Current Status

This version is a snapshot release. It includes all files required for a clean build,
including Memory, HAL, String, NxVector, tests, benchmarks, evidence reports, RFCs,
and developer scripts.
