#ifndef NEXIORA_NCOS_PACKAGE_APPLY_H
#define NEXIORA_NCOS_PACKAGE_APPLY_H

#include "Nexiora/NCOS/NxPackageManager.h"

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

int NxPackageManager_LogHasWarnings(const char* log_path, int* out_warning_count);
const char* NxPackageManager_ApplyPhaseName(NxPackageApplyPhase phase);

#ifdef __cplusplus
}
#endif

#endif
