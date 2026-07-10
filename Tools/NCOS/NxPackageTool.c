#include "Nexiora/NCOS/NxPackageManager.h"

#include <stdio.h>
#include <string.h>

static void nx_print_usage(void)
{
    puts("Nexiora Package Manager");
    puts("");
    puts("Uso:");
    puts("  nexiora_package verify <package_dir>");
    puts("  nexiora_package install <package_dir>");
    puts("  nexiora_package status <package_id>");
    puts("  nexiora_package deps <package_dir>");
    puts("  nexiora_package rollback <package_id>");
}

int main(int argc, char** argv)
{
    NxPackageInstallResult install_result;

    if (argc < 3) {
        nx_print_usage();
        return 1;
    }

    if (strcmp(argv[1], "verify") == 0) {
        NxPackageVerifyResult verify_result;
        int ok = NxPackageManager_VerifyPackage(argv[2], &verify_result);
        printf("================================================\n");
        printf(" NEXIORA - Package Manager\n");
        printf("================================================\n\n");
        printf("Accion              : verify\n");
        printf("Package ID          : %s\n", verify_result.package_id[0] != '\0' ? verify_result.package_id : "desconocido");
        printf("Version             : %s\n", verify_result.package_version[0] != '\0' ? verify_result.package_version : "desconocida");
        printf("Estado              : %s\n", ok ? "VALID" : "INVALID");
        printf("Declarados          : %d\n", verify_result.files_declared);
        printf("Payload encontrados : %d\n", verify_result.payload_files_found);
        printf("Payload faltantes   : %d\n", verify_result.payload_files_missing);
        printf("Mensaje             : %s\n", verify_result.message);
        return ok ? 0 : 2;
    }

    if (strcmp(argv[1], "deps") == 0) {
        NxPackageVerifyResult dependency_result;
        int ok = NxPackageManager_VerifyDependencies(".", argv[2], &dependency_result);
        printf("================================================\n");
        printf(" NEXIORA - Package Dependency Resolver\n");
        printf("================================================\n\n");
        printf("Package ID             : %s\n", dependency_result.package_id[0] != '\0' ? dependency_result.package_id : "desconocido");
        printf("Estado                 : %s\n", ok ? "SATISFIED" : "BLOCKED");
        printf("Dependencias declaradas: %d\n", dependency_result.dependencies_declared);
        printf("Dependencias instaladas: %d\n", dependency_result.dependencies_satisfied);
        printf("Dependencias faltantes : %d\n", dependency_result.dependencies_missing);
        printf("Mensaje                : %s\n", dependency_result.message);
        return ok ? 0 : 5;
    }

    if (strcmp(argv[1], "install") == 0) {
        int ok = NxPackageManager_Install(".", argv[2], &install_result);
        printf("================================================\n");
        printf(" NEXIORA - Package Manager\n");
        printf("================================================\n\n");
        printf("Accion          : install\n");
        printf("Package ID      : %s\n", install_result.package_id);
        printf("Version         : %s\n", install_result.package_version);
        printf("Estado          : %s\n", ok ? "INSTALLED" : "FAILED");
        printf("Declarados      : %d\n", install_result.files_declared);
        printf("Instalados      : %d\n", install_result.files_installed);
        printf("Omitidos        : %d\n", install_result.files_skipped);
        printf("Backups         : %d\n", install_result.files_backed_up);
        printf("Rollback        : %d\n", install_result.files_rolled_back);
        printf("Transaccion     : %s\n", install_result.transaction_path);
        printf("Registro        : %s\n", install_result.registry_path);
        printf("Log             : %s\n", install_result.install_log_path);
        return ok ? 0 : 3;
    }

    if (strcmp(argv[1], "rollback") == 0) {
        int ok = NxPackageManager_Rollback(".", argv[2], &install_result);
        printf("================================================\n");
        printf(" NEXIORA - Transaction Rollback\n");
        printf("================================================\n\n");
        printf("Package ID       : %s\n", install_result.package_id);
        printf("Estado           : %s\n", ok ? "ROLLED BACK" : "FAILED");
        printf("Archivos revertidos: %d\n", install_result.files_rolled_back);
        printf("Transaccion      : %s\n", install_result.transaction_path);
        return ok ? 0 : 6;
    }

    if (strcmp(argv[1], "status") == 0) {
        int ok = NxPackageManager_Status(".", argv[2], &install_result);
        printf("================================================\n");
        printf(" NEXIORA - Package Manager\n");
        printf("================================================\n\n");
        printf("Accion          : status\n");
        printf("Package ID      : %s\n", install_result.package_id);
        printf("Estado          : %s\n", ok ? "INSTALLED" : "NOT INSTALLED");
        printf("Registro        : %s\n", install_result.registry_path);
        return ok ? 0 : 4;
    }

    nx_print_usage();
    return 1;
}
