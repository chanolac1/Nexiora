#ifndef NEXIORA_NCOS_PACKAGE_APPLY_H
#define NEXIORA_NCOS_PACKAGE_APPLY_H

#include "Nexiora/NCOS/NxPackageManager.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxPackageApplyPhase
{
    NX_PACKAGE_APPLY_NONE = 0,
    NX_PACKAGE_APPLY_DISCOVERY,
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

typedef struct NxPackageDiscoveryResult
{
    char package_dir[512];
    char package_id[128];
    char package_version[64];
    int directories_scanned;
    int valid_candidates;
    int blocked_candidates;
    int installed_candidates;
    int found;
    char message[256];
} NxPackageDiscoveryResult;

int NxPackageManager_LogHasWarnings(const char* log_path, int* out_warning_count);
const char* NxPackageManager_ApplyPhaseName(NxPackageApplyPhase phase);

/* Finds the newest valid, dependency-satisfied and not-yet-installed package. */
int NxPackageManager_DiscoverPendingPackage(const char* repo_root,
                                            NxPackageDiscoveryResult* out_result);

/* Discovers and applies one package. Returns success only after full certification. */
int NxPackageManager_ApplyNext(const char* repo_root,
                               NxPackageDiscoveryResult* out_discovery,
                               NxPackageApplyResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
