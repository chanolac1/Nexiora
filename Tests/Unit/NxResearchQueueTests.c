#include "Nexiora/Research/NxResearchQueue.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    NxResearchQueue queue;
    NxResearchQueueItem storage[4];
    NxResearchQueueItem selected;
    unsigned int low_id;
    unsigned int high_id;

    if (NxResearchQueue_Init(&queue, storage, 4) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("queue init failed\n");
        return 1;
    }

    if (NxResearchQueue_Enqueue(&queue, "slow-vector", 10, 2, &low_id) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("enqueue low failed\n");
        return 1;
    }

    if (NxResearchQueue_Enqueue(&queue, "fast-vector", 90, 1, &high_id) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("enqueue high failed\n");
        return 1;
    }

    if (NxResearchQueue_Count(&queue) != 2)
    {
        printf("queue count mismatch\n");
        return 1;
    }

    if (NxResearchQueue_SelectNext(&queue, &selected) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("select failed\n");
        return 1;
    }

    if (selected.id != high_id || strcmp(selected.experiment_id, "fast-vector") != 0)
    {
        printf("priority selection failed\n");
        return 1;
    }

    if (NxResearchQueue_MarkCompleted(&queue, selected.id) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("mark completed failed\n");
        return 1;
    }

    if (NxResearchQueue_SelectNext(&queue, &selected) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("select second failed\n");
        return 1;
    }

    if (selected.id != low_id)
    {
        printf("fifo tie fallback failed\n");
        return 1;
    }

    return 0;
}
