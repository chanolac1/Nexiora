#ifndef NEXIORA_NCOS_PACKAGE_APPLY_H
#define NEXIORA_NCOS_PACKAGE_APPLY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxPackageApplyPhase
{
    NX_PACKAGE_APPLY_NONE = 0,
    NX_PACKAGE_APPLY_VERIFY,
    NX_PACKAGE_APPLY_DEPENDENCIES,
    NX_PACKAGE_APPLY_INSTALL,
    NX_PACKAGE_APPLY_CONFIGURE,
    NX_PACKAGE_APPLY_BUILD,
    NX_PACKAGE_APPLY_WARNING_GATE,
    NX_PACKAGE_APPLY_TESTS,
    NX_PACKAGE_APPLY_DOCUMENTATION,
    NX_PACKAGE_APPLY_COMPLETE
} NxPackageApplyPhase;

typedef struct NxPackageApplyResult
{
    char package_id[128];
    char package_version[64];
    char transaction_id[32];
    char apply_log_path[512];
    char failed_phase[64];
    char message[256];
    int verify_ok;
    int dependencies_ok;
    int install_ok;
    int configure_ok;
    int build_ok;
    int warning_gate_ok;
    int tests_ok;
    int documentation_ok;
    int rollback_performed;
    int success;
} NxPackageApplyResult;

int NxPackageManager_Apply(const char* repo_root,
                           const char* package_dir,
                           NxPackageApplyResult* out_result);

int NxPackageManager_LogHasWarnings(const char* log_path, int* out_warning_count);
const char* NxPackageManager_ApplyPhaseName(NxPackageApplyPhase phase);

#ifdef __cplusplus
}
#endif

#endif
