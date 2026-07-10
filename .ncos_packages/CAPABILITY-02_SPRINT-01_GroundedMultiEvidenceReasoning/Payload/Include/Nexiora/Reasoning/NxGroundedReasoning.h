#ifndef NEXIORA_REASONING_NX_GROUNDED_REASONING_H
#define NEXIORA_REASONING_NX_GROUNDED_REASONING_H

#include <stddef.h>

#define NX_GR_MAX_PATH 1024U
#define NX_GR_MAX_ANSWER 4096U
#define NX_GR_MAX_EVIDENCE 5U
#define NX_GR_MAX_TEXT 1024U
#define NX_GR_MAX_EXPLANATION 2048U

typedef enum NxGroundedReasoningStatus {
    NX_GR_OK = 0,
    NX_GR_INVALID_ARGUMENT = 1,
    NX_GR_SOURCE_NOT_FOUND = 2,
    NX_GR_PARSE_ERROR = 3,
    NX_GR_INSUFFICIENT_EVIDENCE = 4,
    NX_GR_IO_ERROR = 5
} NxGroundedReasoningStatus;

typedef struct NxGroundedEvidence {
    unsigned int chunk_id;
    unsigned int score;
    char source[NX_GR_MAX_PATH];
    char text[NX_GR_MAX_TEXT];
} NxGroundedEvidence;

typedef struct NxGroundedAnswer {
    NxGroundedReasoningStatus status;
    unsigned int confidence;
    unsigned int evidence_count;
    unsigned int contradiction_count;
    NxGroundedEvidence evidence[NX_GR_MAX_EVIDENCE];
    char answer[NX_GR_MAX_ANSWER];
    char explanation[NX_GR_MAX_EXPLANATION];
    char limitations[NX_GR_MAX_TEXT];
} NxGroundedAnswer;

NxGroundedReasoningStatus NxGroundedReasoning_Ask(
    const char* evidence_path,
    const char* question,
    NxGroundedAnswer* out_answer);

const char* NxGroundedReasoning_StatusName(NxGroundedReasoningStatus status);

#endif
