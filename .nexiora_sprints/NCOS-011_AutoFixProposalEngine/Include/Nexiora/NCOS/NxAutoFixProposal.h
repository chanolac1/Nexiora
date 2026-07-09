#ifndef NEXIORA_NCOS_NX_AUTO_FIX_PROPOSAL_H
#define NEXIORA_NCOS_NX_AUTO_FIX_PROPOSAL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxAutoFixSuggestion {
    char category[64];
    char evidence[512];
    char action[512];
    int confidence;
} NxAutoFixSuggestion;

typedef struct NxAutoFixProposal {
    int success;
    int suggestion_count;
    NxAutoFixSuggestion suggestions[32];
    char summary[1024];
    char proposal_path[512];
} NxAutoFixProposal;

int NxAutoFixProposal_CreateFromReport(const char* knowledge_root,
                                       const char* proposal_id,
                                       const char* analysis_report_path,
                                       NxAutoFixProposal* out);

#ifdef __cplusplus
}
#endif

#endif
