# NCOS-021 Native Package Apply Workflow

This package bootstraps the native `apply` command. It intentionally contains no PowerShell installation script.

After this one-time Package Manager upgrade is installed and `nexiora_package` is rebuilt, every future package is applied with a single native command:

`nexiora_package.exe apply <package_dir>`
