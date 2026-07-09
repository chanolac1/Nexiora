#include "Nexiora/NCOS/NxCompilerEngine.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_compile.exe run <id> <comando>\n");
    printf("\nEjemplo:\n");
    printf("  nexiora_compile.exe run configure \"cmake --preset windows-msvc-release\"\n");
}

int main(int argc, char** argv)
{
    char command[1024];
    int i;
    NxCompilerRunResult result;

    if (argc < 4 || strcmp(argv[1], "run") != 0)
    {
        usage();
        return 1;
    }

    command[0] = '\0';
    for (i = 3; i < argc; ++i)
    {
        if (i > 3) strncat(command, " ", sizeof(command) - strlen(command) - 1);
        strncat(command, argv[i], sizeof(command) - strlen(command) - 1);
    }

    printf("================================================\n");
    printf(" NEXIORA - Compiler Engine\n");
    printf("================================================\n\n");
    printf("Run ID : %s\n", argv[2]);
    printf("Comando: %s\n\n", command);

    (void)NxCompiler_Run(".", command, argv[2], &result);

    printf("Estado       : %s\n", NxCompiler_StatusText(result.status));
    printf("Exit code    : %d\n", result.exit_code);
    printf("Warnings     : %d\n", result.saw_warning);
    printf("Errores      : %d\n", result.saw_error);
    printf("Bytes log    : %lu\n", (unsigned long)result.bytes_captured);
    printf("Log          : %s\n", result.log_path);
    printf("Resumen      : %s\n", result.summary);

    return result.status == NX_COMPILER_RUN_OK ? 0 : 2;
}
