#include "Nexiora/Research/NxTopicInvestigation.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    char topic[256];
    size_t i;
    NxTopicInvestigationResult result;
    NxTopicInvestigationStatus status;

    if (argc < 2)
    {
        printf("Uso:\n");
        printf("  nexiora_investigate SQLite\n");
        printf("  nexiora_investigate \"Memory Allocator\"\n");
        return 1;
    }

    topic[0] = '\0';
    for (i = 1; i < (size_t)argc; ++i)
    {
        if (i > 1)
        {
            (void)strncat(topic, " ", sizeof(topic) - strlen(topic) - 1U);
        }
        (void)strncat(topic, argv[i], sizeof(topic) - strlen(topic) - 1U);
    }

    status = NxTopicInvestigation_Run(".", topic, stdout, &result);
    if (status != NX_TOPIC_INVESTIGATION_OK)
    {
        fprintf(stderr, "Investigacion fallida: %s\n", NxTopicInvestigation_StatusToString(status));
        return 2;
    }

    return 0;
}
