#ifndef NEXIORA_RESEARCH_NX_KNOWLEDGE_GAP_RESEARCH_H
#define NEXIORA_RESEARCH_NX_KNOWLEDGE_GAP_RESEARCH_H

#include "Nexiora/Reasoning/NxGroundedReasoning.h"

#define NX_KGR_MAX_PATH 1024U
#define NX_KGR_MAX_TEXT 2048U
#define NX_KGR_MAX_QUERY 512U
#define NX_KGR_MAX_QUERIES 5U

typedef enum NxKnowledgeGapStatus {
    NX_KGR_SUFFICIENT = 0,
    NX_KGR_GAP_OPENED = 1,
    NX_KGR_INVALID_ARGUMENT = 2,
    NX_KGR_IO_ERROR = 3,
    NX_KGR_REASONING_ERROR = 4
} NxKnowledgeGapStatus;

typedef struct NxKnowledgeGapPlan {
    NxKnowledgeGapStatus status;
    unsigned int confidence;
    unsigned int query_count;
    char subject[NX_KGR_MAX_TEXT];
    char original_question[NX_KGR_MAX_TEXT];
    char gap_reason[NX_KGR_MAX_TEXT];
    char queries[NX_KGR_MAX_QUERIES][NX_KGR_MAX_QUERY];
    char allowed_sources[NX_KGR_MAX_TEXT];
    char success_criteria[NX_KGR_MAX_TEXT];
    char plan_path[NX_KGR_MAX_PATH];
} NxKnowledgeGapPlan;

NxKnowledgeGapStatus NxKnowledgeGapResearch_Assess(
    const char* evidence_path,
    const char* subject,
    const char* question,
    const char* plan_path,
    NxKnowledgeGapPlan* out_plan);

const char* NxKnowledgeGapResearch_StatusName(NxKnowledgeGapStatus status);

#endif
