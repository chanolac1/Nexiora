#include "Nexiora/NCOS/NxPackageManager.h"
#include "Nexiora/NCOS/NxPackageApply.h"

#include <stdio.h>
#include <string.h>

static void nx_print_usage(void)
{
    puts("Nexiora Package Manager");
    puts("");
    puts("Uso:");
    puts("  nexiora_package");
    puts("  nexiora_package apply");
    puts("  nexiora_package apply <package_dir>");
    puts("  nexiora_package verify <package_dir>");
    puts("  nexiora_package install <package_dir>");
    puts("  nexiora_package status <package_id>");
    puts("  nexiora_package deps <package_dir>");
    puts("  nexiora_package rollback <package_id>");
    puts("  nexiora_package history <package_id>");
    puts("  nexiora_package rollback-tx <package_id> <transaction_id>");
}

static int nx_print_apply_result(int ok,
                                 const NxPackageDiscoveryResult* discovery,
                                 const NxPackageApplyResult* apply_result)
{
    printf("================================================\n");
    printf(" NEXIORA - Native Package Apply\n");
    printf("================================================\n\n");
    if (discovery != NULL) {
        printf("Discovery        : %s\n", discovery->found ? "FOUND" : "NONE");
        printf("Scanned          : %d\n", discovery->directories_scanned);
        printf("Candidates       : %d\n", discovery->valid_candidates);
        printf("Blocked          : %d\n", discovery->blocked_candidates);
        printf("Already installed: %d\n", discovery->installed_candidates);
        printf("Selected package : %s\n", discovery->package_dir);
    }
    printf("Package ID       : %s\n", apply_result->package_id);
    printf("Version          : %s\n", apply_result->package_version);
    printf("Estado           : %s\n", ok ? "APPLIED" : "FAILED");
    printf("Verify           : %s\n", apply_result->verify_passed ? "PASS" : "FAIL");
    printf("Dependencies     : %s\n", apply_result->dependencies_passed ? "PASS" : "FAIL");
    printf("Install          : %s\n", apply_result->install_passed ? "PASS" : "FAIL");
    printf("Configure        : %s\n", apply_result->configure_passed ? "PASS" : "FAIL");
    printf("Build            : %s\n", apply_result->build_passed ? "PASS" : "FAIL");
    printf("Warning gate     : %s\n", apply_result->warning_gate_passed ? "PASS" : "FAIL");
    printf("Tests            : %s\n", apply_result->tests_passed ? "PASS" : "FAIL");
    printf("Documentation    : %s\n", apply_result->documentation_passed ? "PASS" : "FAIL");
    printf("Artifacts        : %s\n", apply_result->artifacts_passed ? "PASS" : "FAIL");
    printf("QA repeatability : %s\n", apply_result->qa_repeatability_passed ? "PASS" : "FAIL");
    printf("Certification    : %s\n", apply_result->certification_generated ? "GENERATED" : "MISSING");
    printf("Rollback         : %s\n", apply_result->rolled_back ? "DONE" : "NOT REQUIRED");
    printf("Failed phase     : %s\n", apply_result->failed_phase);
    printf("Transaction ID   : %s\n", apply_result->transaction_id);
    printf("Apply log        : %s\n", apply_result->apply_log_path);
    printf("Certification    : %s\n", apply_result->certification_report_path);
    printf("Readable report  : %s\n", apply_result->certification_text_path);
    printf("Git recommendation: %s\n", apply_result->release_recommendation);
    printf("Message          : %s\n", apply_result->message);
    return ok ? 0 : 9;
}

int main(int argc, char** argv)
{
    NxPackageInstallResult install_result;

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "apply") == 0)) {
        NxPackageDiscoveryResult discovery;
        NxPackageApplyResult apply_result;
        int ok;
        memset(&discovery, 0, sizeof(discovery));
        memset(&apply_result, 0, sizeof(apply_result));
        ok = NxPackageManager_ApplyNext(".", &discovery, &apply_result);
        return nx_print_apply_result(ok, &discovery, &apply_result);
    }

    if (argc < 3) {
        nx_print_usage();
        return 1;
    }

    if (strcmp(argv[1], "apply") == 0) {
        NxPackageApplyResult apply_result;
        int ok;
        memset(&apply_result, 0, sizeof(apply_result));
        ok = NxPackageManager_Apply(".", argv[2], &apply_result);
        return nx_print_apply_result(ok, NULL, &apply_result);
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
        printf("Transaction ID  : %s\n", install_result.transaction_id);
        printf("Historial       : %s\n", install_result.history_path);
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


    if (strcmp(argv[1], "history") == 0) {
        NxPackageHistoryResult history;
        int ok = NxPackageManager_History(".", argv[2], &history);
        printf("================================================\n");
        printf(" NEXIORA - Package History\n");
        printf("================================================\n\n");
        printf("Package ID : %s\n", history.package_id);
        printf("Estado     : %s\n", ok ? "AVAILABLE" : "EMPTY");
        printf("Entradas   : %d\n", history.entries);
        printf("Indice     : %s\n", history.history_path);
        return ok ? 0 : 7;
    }

    if (strcmp(argv[1], "rollback-tx") == 0) {
        int ok;
        if (argc < 4) {
            nx_print_usage();
            return 1;
        }
        ok = NxPackageManager_RollbackTransaction(".", argv[2], argv[3], &install_result);
        printf("================================================\n");
        printf(" NEXIORA - Historical Transaction Rollback\n");
        printf("================================================\n\n");
        printf("Package ID        : %s\n", install_result.package_id);
        printf("Transaction ID    : %s\n", install_result.transaction_id);
        printf("Estado            : %s\n", ok ? "ROLLED BACK" : "FAILED");
        printf("Archivos revertidos: %d\n", install_result.files_rolled_back);
        printf("Transaccion       : %s\n", install_result.transaction_path);
        return ok ? 0 : 8;
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
