#include "Nexiora/Research/NxKnowledgeStore.h"

#include <stdio.h>

int main(void)
{
    NxKnowledgeAnswer answer;
    char text[2048];

    if (NxKnowledgeStore_Query(".", "SQLite", &answer) != NX_KS_OK)
    {
        printf("Expected SQLite knowledge.\n");
        return 1;
    }

    if (answer.confidence_percent < 80)
    {
        printf("Expected high confidence.\n");
        return 1;
    }

    if (NxKnowledgeStore_FormatAnswerSpanish(&answer, text, sizeof(text)) != NX_KS_OK)
    {
        printf("Expected formatted answer.\n");
        return 1;
    }

    return 0;
}
