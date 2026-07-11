#ifndef NEXIORA_REASONING_NX_UNIFIED_COGNITIVE_DIALOGUE_H
#define NEXIORA_REASONING_NX_UNIFIED_COGNITIVE_DIALOGUE_H

#include "Nexiora/Reasoning/NxConversationalContext.h"
#include "Nexiora/Reasoning/NxContradictionHypothesis.h"
#include "Nexiora/Research/NxKnowledgeGapResearch.h"

#define NX_UCD_MAX_PATH 1024U
#define NX_UCD_MAX_TEXT 4096U

typedef enum NxUcdStatus {
    NX_UCD_OK = 0,
    NX_UCD_GAP_OPENED = 1,
    NX_UCD_CONTRADICTORY = 2,
    NX_UCD_INVALID_ARGUMENT = 3,
    NX_UCD_IO_ERROR = 4,
    NX_UCD_REASONING_ERROR = 5
} NxUcdStatus;

typedef enum NxUcdMode {
    NX_UCD_MODE_ANSWER = 1,
    NX_UCD_MODE_CLAIM = 2
} NxUcdMode;

typedef struct NxUcdResult {
    NxUcdStatus status;
    NxUcdMode mode;
    unsigned int turn_index;
    unsigned int confidence;
    unsigned int evidence_count;
    unsigned int contradiction_count;
    char active_subject[NX_UCD_MAX_TEXT];
    char resolved_input[NX_UCD_MAX_TEXT];
    char answer[NX_UCD_MAX_TEXT];
    char explanation[NX_UCD_MAX_TEXT];
    char limitations[NX_UCD_MAX_TEXT];
    char gap_plan_path[NX_UCD_MAX_PATH];
} NxUcdResult;

NxUcdStatus NxUnifiedCognitiveDialogue_Create(
    const char* session_path,
    const char* evidence_path,
    const char* initial_subject);

NxUcdStatus NxUnifiedCognitiveDialogue_Ask(
    const char* session_path,
    const char* question,
    NxUcdResult* out_result);

NxUcdStatus NxUnifiedCognitiveDialogue_EvaluateClaim(
    const char* session_path,
    const char* claim,
    NxUcdResult* out_result);

const char* NxUnifiedCognitiveDialogue_StatusName(NxUcdStatus status);

#endif
