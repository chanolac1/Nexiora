#ifndef NEXIORA_REASONING_NX_CONTRADICTION_HYPOTHESIS_H
#define NEXIORA_REASONING_NX_CONTRADICTION_HYPOTHESIS_H

#include <stddef.h>

#define NX_CH_MAX_PATH 1024U
#define NX_CH_MAX_TEXT 1024U
#define NX_CH_MAX_HYPOTHESES 4U
#define NX_CH_MAX_EVIDENCE 8U
#define NX_CH_MAX_EXPLANATION 4096U

typedef enum NxChStatus {
    NX_CH_OK = 0,
    NX_CH_INVALID_ARGUMENT = 1,
    NX_CH_SOURCE_NOT_FOUND = 2,
    NX_CH_PARSE_ERROR = 3,
    NX_CH_INSUFFICIENT_EVIDENCE = 4,
    NX_CH_CONTRADICTORY_EVIDENCE = 5
} NxChStatus;

typedef enum NxChStance {
    NX_CH_STANCE_SUPPORT = 1,
    NX_CH_STANCE_OPPOSE = 2,
    NX_CH_STANCE_NEUTRAL = 3
} NxChStance;

typedef struct NxChEvidence {
    unsigned int chunk_id;
    unsigned int relevance;
    NxChStance stance;
    char source[NX_CH_MAX_PATH];
    char text[NX_CH_MAX_TEXT];
} NxChEvidence;

typedef struct NxChHypothesis {
    char statement[NX_CH_MAX_TEXT];
    unsigned int support_score;
    unsigned int opposition_score;
    unsigned int confidence;
    unsigned int support_count;
    unsigned int opposition_count;
} NxChHypothesis;

typedef struct NxChResult {
    NxChStatus status;
    unsigned int confidence;
    unsigned int contradiction_count;
    unsigned int evidence_count;
    unsigned int hypothesis_count;
    NxChEvidence evidence[NX_CH_MAX_EVIDENCE];
    NxChHypothesis hypotheses[NX_CH_MAX_HYPOTHESES];
    char conclusion[NX_CH_MAX_EXPLANATION];
    char explanation[NX_CH_MAX_EXPLANATION];
    char missing_evidence[NX_CH_MAX_TEXT];
} NxChResult;

NxChStatus NxContradictionHypothesis_Analyze(
    const char* evidence_path,
    const char* claim,
    NxChResult* out_result);

const char* NxContradictionHypothesis_StatusName(NxChStatus status);
const char* NxContradictionHypothesis_StanceName(NxChStance stance);

#endif
