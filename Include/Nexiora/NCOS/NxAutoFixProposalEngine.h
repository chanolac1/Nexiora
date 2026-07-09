#ifndef NEXIORA_NCOS_NX_AUTO_FIX_PROPOSAL_ENGINE_H
#define NEXIORA_NCOS_NX_AUTO_FIX_PROPOSAL_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxAutoFixProposalSuggestion {
    char category[64];
    char evidence[512];
    char action[512];
    int confidence;
} NxAutoFixProposalSuggestion;

typedef struct NxAutoFixProposalResult {
    int success;
    int suggestion_count;
    NxAutoFixProposalSuggestion suggestions[32];
    char summary[1024];
    char proposal_path[512];
} NxAutoFixProposalResult;

int NxAutoFixProposalEngine_CreateFromReport(const char* knowledge_root,
                                             const char* proposal_id,
                                             const char* analysis_report_path,
                                             NxAutoFixProposalResult* out);

#ifdef __cplusplus
}
#endif

#endif
