# Nexiora 0.0.7 "Developer Platform"

Nexiora is a modular, evidence-driven AI platform foundation.
This release consolidates the project into a complete repository snapshot and improves the local developer workflow.

## Promise

> Nexiora never claims to be better. Nexiora demonstrates improvement through reproducible evidence.

## Current NCP Modules

- Runtime lifecycle
- Result/error system
- Memory Manager and Arena
- Logging system
- Hardware Abstraction Layer
- String primitives
- NxVector container
- Benchmark Engine
- Evidence Engine
- Developer Platform scripts

## Windows build

Requirements:

- Windows 10/11 x64
- CMake 3.25+
- Ninja
- Visual Studio 2022 Developer PowerShell or MSYS2 UCRT64 shell with GCC

Recommended workflow:

```powershell
cd D:\Nexiora
Unblock-File .\Scripts\nxbuild.ps1
.\Scripts\nxbuild.ps1 doctor
.\Scripts\nxbuild.ps1 configure
.\Scripts\nxbuild.ps1 build
.\Scripts\nxbuild.ps1 test
.\Scripts\nxbuild.ps1 benchmark containers
```

Full workflow:

```powershell
.\Scripts\nxbuild.ps1 all containers
```

Manual workflow:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
.\Build\windows-msvc-debug\bin\nexiora.exe
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module containers
```

## nxbuild commands

```powershell
.\Scripts\nxbuild.ps1 doctor
.\Scripts\nxbuild.ps1 configure
.\Scripts\nxbuild.ps1 build
.\Scripts\nxbuild.ps1 test
.\Scripts\nxbuild.ps1 benchmark all
.\Scripts\nxbuild.ps1 benchmark memory
.\Scripts\nxbuild.ps1 benchmark hardware
.\Scripts\nxbuild.ps1 benchmark string
.\Scripts\nxbuild.ps1 benchmark containers
.\Scripts\nxbuild.ps1 approve
.\Scripts\nxbuild.ps1 clean
```

Legacy-compatible form:

```powershell
.\Scripts\nxbuild.ps1 benchmark --module containers
```

## Repository layout

```text
Nexiora/
├── RFC/                  Architecture decision documents
├── Docs/                 Build and engineering documentation
├── Include/Nexiora/NCP/  Public C API headers
├── Source/NCP/           NCP implementation
├── Tests/                Unit tests and benchmark entry points
├── Scripts/              Build helper scripts
├── Config/               Future configuration files
├── Models/               Future model resources
├── Datasets/             Future datasets
├── Tools/                Future developer tools
└── Build/                Generated build output
```

## Evidence

Benchmark reports are written to:

```text
Benchmarks/Reports/latest_evidence_report.txt
Benchmarks/History/nexiora_bench_history.csv
Benchmarks/Approvals/
```

Promote a candidate only after reviewing the benchmark output:

```powershell
.\Scripts\nxbuild.ps1 approve
```
