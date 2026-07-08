#include "Nexiora/Conversation/NxConversation.h"

#include <stdio.h>
#include <string.h>

static int expect_intent(const char* input, NxConversationIntent intent)
{
    NxConversationResponse response;
    NxConversationStatus status;

    status = NxConversation_Respond(input, &response);
    if (status != NX_CONVERSATION_OK)
    {
        printf("Unexpected status for '%s': %s\n", input, NxConversation_StatusToString(status));
        return 1;
    }

    if (response.intent != intent)
    {
        printf("Unexpected intent for '%s': got %s\n", input, NxConversation_IntentToString(response.intent));
        return 1;
    }

    if (strlen(response.text) == 0)
    {
        printf("Empty response for '%s'\n", input);
        return 1;
    }

    return 0;
}

int main(void)
{
    NxConversationResponse response;

    if (expect_intent("ayuda", NX_CONVERSATION_INTENT_HELP) != 0) return 1;
    if (expect_intent("como estas", NX_CONVERSATION_INTENT_STATUS) != 0) return 1;
    if (expect_intent("investiga scheduler", NX_CONVERSATION_INTENT_RESEARCH) != 0) return 1;
    if (expect_intent("que aprendiste", NX_CONVERSATION_INTENT_LEARNED) != 0) return 1;
    if (expect_intent("sorprendeme", NX_CONVERSATION_INTENT_SURPRISE) != 0) return 1;
    if (expect_intent("salir", NX_CONVERSATION_INTENT_EXIT) != 0) return 1;

    if (NxConversation_Respond("salir", &response) != NX_CONVERSATION_OK) return 1;
    if (!response.should_exit)
    {
        printf("Expected exit response to set should_exit\n");
        return 1;
    }

    return 0;
}
