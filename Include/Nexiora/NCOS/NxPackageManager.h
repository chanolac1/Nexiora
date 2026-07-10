#ifndef NEXIORA_NCOS_PACKAGE_MANAGER_H
#define NEXIORA_NCOS_PACKAGE_MANAGER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxPackageInstallResult
{
    char package_id[128];
    char package_version[64];
    char registry_path[512];
    char install_log_path[512];
    int files_declared;
    int files_installed;
    int files_skipped;
    int files_backed_up;
    int success;
} NxPackageInstallResult;

int NxPackageManager_Install(const char* repo_root,
                             const char* package_dir,
                             NxPackageInstallResult* out_result);

int NxPackageManager_Status(const char* repo_root,
                            const char* package_id,
                            NxPackageInstallResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
