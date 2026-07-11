# NCOS-021 Validation Evidence

## Strict compilation

`NxPackageManager.c`, `NxPackageTool.c` and `NxPackageManagerTests.c` compiled with C23, `-Wall`, `-Wextra`, `-Wpedantic`, `-Werror`.

Result: 0 errors, 0 warnings.

## Unit test

`NxPackageManagerTests: PASS`

## Native apply integration

Success scenario: verify, dependencies, install, configure, build, warning gate, tests and documentation all passed.

Failure scenario: a simulated compiler warning caused `warning-gate` failure, exit code 9 and rollback of transaction `000002`; the prior installed file content was restored.
