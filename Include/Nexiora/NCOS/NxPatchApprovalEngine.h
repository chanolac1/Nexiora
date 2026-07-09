#ifndef NEXIORA_NCOS_NX_PATCH_APPROVAL_ENGINE_H
#define NEXIORA_NCOS_NX_PATCH_APPROVAL_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxPatchApprovalStatusTag
{
    NX_PATCH_APPROVAL_UNKNOWN = 0,
    NX_PATCH_APPROVAL_PENDING = 1,
    NX_PATCH_APPROVAL_APPROVED = 2,
    NX_PATCH_APPROVAL_REJECTED = 3
} NxPatchApprovalStatus;

typedef struct NxPatchApprovalResultTag
{
    int ok;
    char run_id[128];
    char status[32];
    char approval_path[512];
    char proposal_path[512];
    char reviewer[128];
    char summary[512];
} NxPatchApprovalResult;

int NxPatchApproval_Request(const char* root_path,
                            const char* run_id,
                            const char* proposal_path,
                            NxPatchApprovalResult* out_result);

int NxPatchApproval_Approve(const char* root_path,
                            const char* run_id,
                            const char* reviewer,
                            NxPatchApprovalResult* out_result);

int NxPatchApproval_Reject(const char* root_path,
                           const char* run_id,
                           const char* reviewer,
                           const char* reason,
                           NxPatchApprovalResult* out_result);

int NxPatchApproval_Status(const char* root_path,
                           const char* run_id,
                           NxPatchApprovalResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
