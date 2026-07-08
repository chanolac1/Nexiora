#ifndef NEXIORA_RESEARCH_NX_RESEARCH_QUEUE_H
#define NEXIORA_RESEARCH_NX_RESEARCH_QUEUE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchQueueStatus
{
    NX_RESEARCH_QUEUE_STATUS_OK = 0,
    NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT = 1,
    NX_RESEARCH_QUEUE_STATUS_CAPACITY_EXCEEDED = 2,
    NX_RESEARCH_QUEUE_STATUS_NOT_FOUND = 3,
    NX_RESEARCH_QUEUE_STATUS_EMPTY = 4
} NxResearchQueueStatus;

typedef enum NxResearchQueueItemState
{
    NX_RESEARCH_QUEUE_ITEM_PENDING = 0,
    NX_RESEARCH_QUEUE_ITEM_RUNNING = 1,
    NX_RESEARCH_QUEUE_ITEM_COMPLETED = 2,
    NX_RESEARCH_QUEUE_ITEM_FAILED = 3,
    NX_RESEARCH_QUEUE_ITEM_SKIPPED = 4
} NxResearchQueueItemState;

typedef struct NxResearchQueueItem
{
    unsigned int id;
    char experiment_id[64];
    int priority;
    unsigned int attempts;
    unsigned int max_attempts;
    NxResearchQueueItemState state;
} NxResearchQueueItem;

typedef struct NxResearchQueue
{
    NxResearchQueueItem* items;
    size_t count;
    size_t capacity;
    unsigned int next_id;
} NxResearchQueue;

NxResearchQueueStatus NxResearchQueue_Init(
    NxResearchQueue* queue,
    NxResearchQueueItem* storage,
    size_t capacity);

void NxResearchQueue_Reset(NxResearchQueue* queue);

NxResearchQueueStatus NxResearchQueue_Enqueue(
    NxResearchQueue* queue,
    const char* experiment_id,
    int priority,
    unsigned int max_attempts,
    unsigned int* id_out);

size_t NxResearchQueue_Count(const NxResearchQueue* queue);

NxResearchQueueStatus NxResearchQueue_Get(
    const NxResearchQueue* queue,
    size_t index,
    const NxResearchQueueItem** item_out);

NxResearchQueueStatus NxResearchQueue_SelectNext(
    NxResearchQueue* queue,
    NxResearchQueueItem* item_out);

NxResearchQueueStatus NxResearchQueue_MarkCompleted(
    NxResearchQueue* queue,
    unsigned int id);

NxResearchQueueStatus NxResearchQueue_MarkFailed(
    NxResearchQueue* queue,
    unsigned int id);

NxResearchQueueStatus NxResearchQueue_MarkSkipped(
    NxResearchQueue* queue,
    unsigned int id);

#ifdef __cplusplus
}
#endif

#endif
