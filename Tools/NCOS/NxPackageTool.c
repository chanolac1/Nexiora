#include "Nexiora/NCOS/NxPackageManager.h"

#include <stdio.h>
#include <string.h>

static void nx_print_usage(void)
{
    puts("Nexiora Package Manager");
    puts("");
    puts("Uso:");
    puts("  nexiora_package install <package_dir>");
    puts("  nexiora_package status <package_id>");
}

int main(int argc, char** argv)
{
    NxPackageInstallResult result;

    if (argc < 3) {
        nx_print_usage();
        return 1;
    }

    if (strcmp(argv[1], "install") == 0) {
        int ok = NxPackageManager_Install(".", argv[2], &result);
        printf("================================================\n");
        printf(" NEXIORA - Package Manager\n");
        printf("================================================\n\n");
        printf("Accion          : install\n");
        printf("Package ID      : %s\n", result.package_id);
        printf("Version         : %s\n", result.package_version);
        printf("Estado          : %s\n", ok ? "INSTALLED" : "FAILED");
        printf("Declarados      : %d\n", result.files_declared);
        printf("Instalados      : %d\n", result.files_installed);
        printf("Omitidos        : %d\n", result.files_skipped);
        printf("Backups         : %d\n", result.files_backed_up);
        printf("Registro        : %s\n", result.registry_path);
        printf("Log             : %s\n", result.install_log_path);
        return ok ? 0 : 2;
    }

    if (strcmp(argv[1], "status") == 0) {
        int ok = NxPackageManager_Status(".", argv[2], &result);
        printf("================================================\n");
        printf(" NEXIORA - Package Manager\n");
        printf("================================================\n\n");
        printf("Accion          : status\n");
        printf("Package ID      : %s\n", result.package_id);
        printf("Estado          : %s\n", ok ? "INSTALLED" : "NOT INSTALLED");
        printf("Registro        : %s\n", result.registry_path);
        return ok ? 0 : 3;
    }

    nx_print_usage();
    return 1;
}
