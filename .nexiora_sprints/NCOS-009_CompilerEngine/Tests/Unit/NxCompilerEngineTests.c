#include "Nexiora/NCOS/NxCompilerEngine.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int failures = 0;

static void check(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    NxCompilerRunResult result;
#ifdef _WIN32
    const char* ok_command = "cmd /c echo NCOS_COMPILER_OK";
#else
    const char* ok_command = "printf NCOS_COMPILER_OK";
#endif

#ifdef _WIN32
    (void)system("if exist .ncos_compiler_test rmdir /s /q .ncos_compiler_test");
#else
    (void)system("rm -rf .ncos_compiler_test");
#endif

    check(NxCompiler_Run(".ncos_compiler_test", ok_command, "unit_ok", &result) == NX_COMPILER_RUN_OK, "run should succeed");
    check(result.exit_code == 0, "exit code should be zero");
    check(result.bytes_captured > 0, "should capture output");
    check(strstr(result.log_path, "unit_ok.log") != NULL, "log path should include run id");
    check(result.saw_error == 0, "successful echo should not be flagged as error");
    check(strlen(result.summary) > 0, "summary should be populated");

#ifdef _WIN32
    (void)system("if exist .ncos_compiler_test rmdir /s /q .ncos_compiler_test");
#else
    (void)system("rm -rf .ncos_compiler_test");
#endif

    return failures == 0 ? 0 : 1;
}
