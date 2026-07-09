#ifndef NEXIORA_NCOS_NX_SAFE_PATCH_ENGINE_H
#define NEXIORA_NCOS_NX_SAFE_PATCH_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxSafePatchResult
{
    int ok;
    int proposed_changes;
    int requires_human_approval;
    char run_id[128];
    char proposal_path[512];
    char summary[512];
} NxSafePatchResult;

int NxSafePatch_CreateProposal(const char* root_path,
                               const char* run_id,
                               const char* analysis_or_proposal_path,
                               NxSafePatchResult* out_result);

#ifdef __cplusplus
}
#endif

#endif
