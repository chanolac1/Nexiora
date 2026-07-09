#include "Nexiora/Research/NxLearningCore.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    char buffer[2048];
    NxLearningCoreStatus status;

    status = NxLearningCore_Query("topic_that_does_not_exist_yet", buffer, sizeof(buffer));
    if (status != NX_LEARNING_CORE_NOT_FOUND)
    {
        printf("Expected not found for unknown topic.\n");
        return 1;
    }

    if (strstr(buffer, "No tengo conocimiento suficiente") == 0)
    {
        printf("Expected helpful not-found message.\n");
        return 1;
    }

    if (NxLearningCore_StatusToString(NX_LEARNING_CORE_OK) == 0)
    {
        printf("Expected status string.\n");
        return 1;
    }

    return 0;
}
