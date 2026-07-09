#include "Nexiora/Research/NxTopicInvestigation.h"
#include "Nexiora/Research/NxKnowledgeStore.h"

#include <stdio.h>

int main(void)
{
    NxTopicInvestigationResult result;
    NxKnowledgeAnswer answer;

    if (NxTopicInvestigation_Run(".", "SQLite", NULL, &result) != NX_TOPIC_INVESTIGATION_OK)
    {
        printf("Expected investigation to run.\n");
        return 1;
    }

    if (result.concept_count < 5 || result.confidence_percent < 80)
    {
        printf("Unexpected investigation result.\n");
        return 1;
    }

    if (NxKnowledgeStore_Query(".", "SQLite", &answer) != NX_KS_OK)
    {
        printf("Expected learned SQLite answer.\n");
        return 1;
    }

    return 0;
}
