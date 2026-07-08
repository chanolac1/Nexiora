#ifndef NEXIORA_CONVERSATION_NX_CONVERSATION_H
#define NEXIORA_CONVERSATION_NX_CONVERSATION_H

#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxConversationStatus
{
    NX_CONVERSATION_OK = 0,
    NX_CONVERSATION_INVALID_ARGUMENT = 1,
    NX_CONVERSATION_OUTPUT_TOO_SMALL = 2
} NxConversationStatus;

typedef enum NxConversationIntent
{
    NX_CONVERSATION_INTENT_UNKNOWN = 0,
    NX_CONVERSATION_INTENT_HELP = 1,
    NX_CONVERSATION_INTENT_STATUS = 2,
    NX_CONVERSATION_INTENT_RESEARCH = 3,
    NX_CONVERSATION_INTENT_SESSIONS = 4,
    NX_CONVERSATION_INTENT_LAST_SESSION = 5,
    NX_CONVERSATION_INTENT_LEARNED = 6,
    NX_CONVERSATION_INTENT_RECOMMENDATIONS = 7,
    NX_CONVERSATION_INTENT_SURPRISE = 8,
    NX_CONVERSATION_INTENT_EXIT = 9
} NxConversationIntent;

typedef struct NxConversationResponse
{
    NxConversationIntent intent;
    int should_exit;
    unsigned confidence_percent;
    char text[2048];
} NxConversationResponse;

NxConversationStatus NxConversation_Respond(
    const char* input,
    NxConversationResponse* response_out);

NxConversationStatus NxConversation_RunInteractive(FILE* input, FILE* output);

const char* NxConversation_StatusToString(NxConversationStatus status);
const char* NxConversation_IntentToString(NxConversationIntent intent);

#ifdef __cplusplus
}
#endif

#endif
