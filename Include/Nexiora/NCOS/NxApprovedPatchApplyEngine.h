#ifndef NEXIORA_NCOS_NX_APPROVED_PATCH_APPLY_ENGINE_H
#define NEXIORA_NCOS_NX_APPROVED_PATCH_APPLY_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxApprovedPatchApplyResultTag
{
    int ok;
    int approved;
    char run_id[128];
    char status[32];
    char approval_path[512];
    char proposal_path[512];
    char application_path[512];
    char summary[512];
} NxApprovedPatchApplyResult;

int NxApprovedPatchApply_Run(const char* root_path,
                             const char* run_id,
                             NxApprovedPatchApplyResult* out_result);

int NxApprovedPatchApply_Status(const char* root_path,
                                const char* run_id,
                                NxApprovedPatchApplyResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
