# NCOS-018 Package Dependency Resolution

`manifest.npkg` may declare one or more dependencies:

```text
requires=NCOS-017 Package Verification Workflow
```

Before installation, the package manager normalizes each dependency ID and checks its local registry under `Knowledge/NCOS/Packages/<id>/registry.txt`. Installation is blocked when any dependency is missing.

Commands:

```powershell
nexiora_package.exe deps <package_dir>
nexiora_package.exe install <package_dir>
```
