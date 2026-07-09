# NCOS-009 Repair

Fixes `NxCompilerEngineTests` by ensuring the compiler engine creates the sandbox root directory before nested Knowledge/NCOS/Compiler folders.

Also updates the unit test to use a portable `echo NCOS_COMPILER_OK` command.
