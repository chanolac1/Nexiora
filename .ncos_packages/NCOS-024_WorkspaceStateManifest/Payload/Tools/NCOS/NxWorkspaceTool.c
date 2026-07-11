#include "Nexiora/NCOS/NxWorkspaceEngine.h"
#include "Nexiora/NCOS/NxWorkspaceManifest.h"

#include <stdio.h>
#include <string.h>

static void nx_print_info(const NxWorkspaceInfo* info)
{
    printf("Workspace ID : %s\n", info->id);
    printf("Estado       : %s\n", NxWorkspace_StateName(info->state));
    printf("Solicitados  : %zu\n", info->files_requested);
    printf("Copiados     : %zu\n", info->files_copied);
    printf("Omitidos     : %zu\n", info->files_skipped);
    printf("Ruta         : %s\n", info->path);
    printf("Manifest     : %s\n", info->manifest_path);
    printf("Action log   : %s\n", info->log_path);
}

static void nx_print_manifest(const NxWorkspaceManifestResult* result)
{
    printf("status=%s\n", NxWorkspaceManifest_StatusName(result->status));
    printf("manifest=%s\n", result->manifest_path);
    printf("git_branch=%s\n", result->git_branch);
    printf("git_commit=%s\n", result->git_commit);
    printf("git_dirty=%s\n", result->git_dirty ? "true" : "false");
    printf("targets=%zu\n", result->target_count);
    printf("tests=%zu\n", result->test_count);
    printf("executables=%zu\n", result->executable_count);
    printf("installed_packages=%zu\n", result->installed_package_count);
    printf("pending_packages=%zu\n", result->pending_package_count);
    printf("blocked_packages=%zu\n", result->blocked_package_count);
    printf("documentation_checked=%zu\n", result->documentation_checked);
    printf("documentation_missing=%zu\n", result->documentation_missing);
    printf("differences=%zu\n", result->differences);
    printf("message=%s\n", result->message);
}

static void nx_usage(void)
{
    puts("Nexiora Workspace Engine");
    puts("");
    puts("Uso:");
    puts("  nexiora_workspace snapshot <root> [workspace.nxws]");
    puts("  nexiora_workspace validate <root> [workspace.nxws]");
    puts("  nexiora_workspace diff <root> <workspace.nxws>");
    puts("  nexiora_workspace create <id> <archivo_relativo> [archivo_relativo ...]");
    puts("  nexiora_workspace status <id>");
    puts("  nexiora_workspace close <id>");
    puts("  nexiora_workspace clean <id>");
}

int main(int argc, char** argv)
{
    NxWorkspaceInfo info;
    NxWorkspaceManifestResult manifest;
    if (argc >= 3 && strcmp(argv[1], "snapshot") == 0) {
        const char* output = argc >= 4 ? argv[3] : "workspace.nxws";
        int ok = NxWorkspaceManifest_Snapshot(argv[2], output, &manifest);
        nx_print_manifest(&manifest);
        return ok ? 0 : 1;
    }
    if (argc >= 3 && strcmp(argv[1], "validate") == 0) {
        const char* input = argc >= 4 ? argv[3] : "workspace.nxws";
        int ok = NxWorkspaceManifest_Validate(argv[2], input, &manifest);
        nx_print_manifest(&manifest);
        return ok ? 0 : 1;
    }
    if (argc >= 4 && strcmp(argv[1], "diff") == 0) {
        int ok = NxWorkspaceManifest_Diff(argv[2], argv[3], &manifest);
        nx_print_manifest(&manifest);
        return ok ? 0 : 1;
    }
    if (argc < 3) {
        nx_usage();
        return 2;
    }
    if (strcmp(argv[1], "create") == 0) {
        const char* const* files;
        size_t count;
        if (argc < 4) { nx_usage(); return 2; }
        files = (const char* const*)&argv[3];
        count = (size_t)(argc - 3);
        if (!NxWorkspace_Create(".", argv[2], files, count, &info)) {
            fprintf(stderr, "No se pudo crear el workspace.\n");
            return 1;
        }
        puts("Workspace autonomo creado.");
        nx_print_info(&info);
        return 0;
    }
    if (strcmp(argv[1], "status") == 0) {
        if (!NxWorkspace_Status(".", argv[2], &info)) { fprintf(stderr, "Workspace no encontrado.\n"); return 1; }
        nx_print_info(&info);
        return 0;
    }
    if (strcmp(argv[1], "close") == 0) {
        if (!NxWorkspace_Close(".", argv[2], &info)) { fprintf(stderr, "No se pudo cerrar el workspace.\n"); return 1; }
        puts("Workspace cerrado.");
        nx_print_info(&info);
        return 0;
    }
    if (strcmp(argv[1], "clean") == 0) {
        if (!NxWorkspace_Clean(".", argv[2])) { fprintf(stderr, "No se pudo limpiar el workspace.\n"); return 1; }
        puts("Workspace eliminado.");
        return 0;
    }
    nx_usage();
    return 2;
}
