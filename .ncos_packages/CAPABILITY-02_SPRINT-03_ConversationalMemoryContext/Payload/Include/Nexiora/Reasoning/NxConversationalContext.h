#ifndef NEXIORA_REASONING_NX_CONVERSATIONAL_CONTEXT_H
#define NEXIORA_REASONING_NX_CONVERSATIONAL_CONTEXT_H
#include "Nexiora/Reasoning/NxGroundedReasoning.h"
#define NX_CC_MAX_PATH 1024U
#define NX_CC_MAX_TEXT 2048U
typedef enum NxCcStatus { NX_CC_OK=0, NX_CC_INVALID_ARGUMENT=1, NX_CC_IO_ERROR=2, NX_CC_REASONING_ERROR=3 } NxCcStatus;
typedef struct NxCcTurnResult {
    NxCcStatus status;
    unsigned int turn_index;
    char resolved_question[NX_CC_MAX_TEXT];
    char active_subject[NX_CC_MAX_TEXT];
    NxGroundedAnswer grounded;
} NxCcTurnResult;
NxCcStatus NxConversationalContext_Create(const char* session_path,const char* evidence_path,const char* initial_subject);
NxCcStatus NxConversationalContext_Ask(const char* session_path,const char* question,NxCcTurnResult* out_result);
const char* NxConversationalContext_StatusName(NxCcStatus status);
#endif
