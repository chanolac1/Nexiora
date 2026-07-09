#include "Nexiora/Research/NxTopicInvestigation.h"

#include <stdio.h>

int main(void)
{
    NxTopicInvestigationResult result;
    NxTopicInvestigationStatus status;

    status = NxTopicInvestigation_Run(".", "SQLite", 0, &result);
    if (status != NX_TOPIC_INVESTIGATION_OK)
    {
        printf("Expected topic investigation OK.\n");
        return 1;
    }

    if (result.concepts_extracted != 10U)
    {
        printf("Expected 10 concepts.\n");
        return 1;
    }

    if (result.relations_created != 30U)
    {
        printf("Expected 30 relations.\n");
        return 1;
    }

    if (result.confidence != 92U)
    {
        printf("Expected confidence 92.\n");
        return 1;
    }

    return 0;
}
