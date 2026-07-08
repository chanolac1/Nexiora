#include "Nexiora/Research/NxResearchQueue.h"

#include <string.h>

static void NxResearchQueue_CopyText(char* dst, size_t dst_size, const char* src)
{
    size_t i = 0;
    if (dst == 0 || dst_size == 0)
    {
        return;
    }
    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }
    while (i + 1 < dst_size && src[i] != '\0')
    {
        dst[i] = src[i];
        ++i;
    }
    dst[i] = '\0';
}

static NxResearchQueueItem* NxResearchQueue_FindMutable(NxResearchQueue* queue, unsigned int id)
{
    size_t i;
    if (queue == 0)
    {
        return 0;
    }
    for (i = 0; i < queue->count; ++i)
    {
        if (queue->items[i].id == id)
        {
            return &queue->items[i];
        }
    }
    return 0;
}

NxResearchQueueStatus NxResearchQueue_Init(
    NxResearchQueue* queue,
    NxResearchQueueItem* storage,
    size_t capacity)
{
    if (queue == 0 || storage == 0 || capacity == 0)
    {
        return NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT;
    }
    queue->items = storage;
    queue->count = 0;
    queue->capacity = capacity;
    queue->next_id = 1;
    memset(storage, 0, sizeof(NxResearchQueueItem) * capacity);
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

void NxResearchQueue_Reset(NxResearchQueue* queue)
{
    if (queue == 0)
    {
        return;
    }
    if (queue->items != 0 && queue->capacity > 0)
    {
        memset(queue->items, 0, sizeof(NxResearchQueueItem) * queue->capacity);
    }
    queue->count = 0;
    queue->next_id = 1;
}

NxResearchQueueStatus NxResearchQueue_Enqueue(
    NxResearchQueue* queue,
    const char* experiment_id,
    int priority,
    unsigned int max_attempts,
    unsigned int* id_out)
{
    NxResearchQueueItem* item;
    if (queue == 0 || queue->items == 0 || experiment_id == 0 || experiment_id[0] == '\0')
    {
        return NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT;
    }
    if (queue->count >= queue->capacity)
    {
        return NX_RESEARCH_QUEUE_STATUS_CAPACITY_EXCEEDED;
    }
    item = &queue->items[queue->count++];
    item->id = queue->next_id++;
    NxResearchQueue_CopyText(item->experiment_id, sizeof(item->experiment_id), experiment_id);
    item->priority = priority;
    item->attempts = 0;
    item->max_attempts = max_attempts == 0 ? 1 : max_attempts;
    item->state = NX_RESEARCH_QUEUE_ITEM_PENDING;
    if (id_out != 0)
    {
        *id_out = item->id;
    }
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

size_t NxResearchQueue_Count(const NxResearchQueue* queue)
{
    return queue == 0 ? 0u : queue->count;
}

NxResearchQueueStatus NxResearchQueue_Get(
    const NxResearchQueue* queue,
    size_t index,
    const NxResearchQueueItem** item_out)
{
    if (queue == 0 || item_out == 0)
    {
        return NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT;
    }
    if (index >= queue->count)
    {
        return NX_RESEARCH_QUEUE_STATUS_NOT_FOUND;
    }
    *item_out = &queue->items[index];
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

NxResearchQueueStatus NxResearchQueue_SelectNext(
    NxResearchQueue* queue,
    NxResearchQueueItem* item_out)
{
    size_t i;
    size_t best_index = 0;
    int found = 0;
    if (queue == 0 || item_out == 0)
    {
        return NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT;
    }
    for (i = 0; i < queue->count; ++i)
    {
        NxResearchQueueItem* candidate = &queue->items[i];
        if (candidate->state != NX_RESEARCH_QUEUE_ITEM_PENDING)
        {
            continue;
        }
        if (candidate->attempts >= candidate->max_attempts)
        {
            continue;
        }
        if (!found || candidate->priority > queue->items[best_index].priority ||
            (candidate->priority == queue->items[best_index].priority && candidate->id < queue->items[best_index].id))
        {
            best_index = i;
            found = 1;
        }
    }
    if (!found)
    {
        return NX_RESEARCH_QUEUE_STATUS_EMPTY;
    }
    queue->items[best_index].state = NX_RESEARCH_QUEUE_ITEM_RUNNING;
    queue->items[best_index].attempts += 1;
    *item_out = queue->items[best_index];
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

NxResearchQueueStatus NxResearchQueue_MarkCompleted(NxResearchQueue* queue, unsigned int id)
{
    NxResearchQueueItem* item = NxResearchQueue_FindMutable(queue, id);
    if (item == 0)
    {
        return queue == 0 ? NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT : NX_RESEARCH_QUEUE_STATUS_NOT_FOUND;
    }
    item->state = NX_RESEARCH_QUEUE_ITEM_COMPLETED;
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

NxResearchQueueStatus NxResearchQueue_MarkFailed(NxResearchQueue* queue, unsigned int id)
{
    NxResearchQueueItem* item = NxResearchQueue_FindMutable(queue, id);
    if (item == 0)
    {
        return queue == 0 ? NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT : NX_RESEARCH_QUEUE_STATUS_NOT_FOUND;
    }
    item->state = item->attempts < item->max_attempts ? NX_RESEARCH_QUEUE_ITEM_PENDING : NX_RESEARCH_QUEUE_ITEM_FAILED;
    return NX_RESEARCH_QUEUE_STATUS_OK;
}

NxResearchQueueStatus NxResearchQueue_MarkSkipped(NxResearchQueue* queue, unsigned int id)
{
    NxResearchQueueItem* item = NxResearchQueue_FindMutable(queue, id);
    if (item == 0)
    {
        return queue == 0 ? NX_RESEARCH_QUEUE_STATUS_INVALID_ARGUMENT : NX_RESEARCH_QUEUE_STATUS_NOT_FOUND;
    }
    item->state = NX_RESEARCH_QUEUE_ITEM_SKIPPED;
    return NX_RESEARCH_QUEUE_STATUS_OK;
}
