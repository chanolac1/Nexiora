# Nexiora 0.0.8 — Research Lab

This snapshot introduces **NRL-0001 — Laboratory Kernel**, the first implementation of the Nexiora Research Lab.

## New in this snapshot

- `Research/` infrastructure
- `LABBOOK/`
- `SPEC/`
- `NxExperiment`
- `NxResearchKernel`
- experiment state machine
- registry writer
- journal writer
- research unit tests
- research benchmark module
- RFC-0016 through RFC-0021
- BOOK updates

## Windows build

```powershell
Unblock-File .\Scripts\nxbuild.ps1
.\Scripts\nxbuild.ps1 clean
.\Scripts\nxbuild.ps1 all research
```

Or manually:

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset debug
ctest --preset debug-tests --output-on-failure
.\Build\windows-msvc-debug\bin\nexiora_bench.exe --iterations 1000000 --module research
```

## Commit

```powershell
git add .
git commit -m "NRL-0001: Add Laboratory Kernel"
git push
```
