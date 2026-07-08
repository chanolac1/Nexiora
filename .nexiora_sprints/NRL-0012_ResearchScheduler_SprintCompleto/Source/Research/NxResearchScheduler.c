#include "Nexiora/Research/NxResearchScheduler.h"

#include <stdlib.h>
#include <string.h>

static int NxResearchScheduler_IsEligible(
    const NxResearchScheduler* scheduler,
    const NxResearchTaskDescriptor* task)
{
    if (scheduler == 0 || task == 0)
    {
        return 0;
    }

    if (task->state == NX_RESEARCH_TASK_STATE_DISABLED ||
        task->state == NX_RESEARCH_TASK_STATE_COMPLETED ||
        task->state == NX_RESEARCH_TASK_STATE_RUNNING)
    {
        return 0;
    }

    if (task->max_runs != 0 && task->completed_runs >= task->max_runs)
    {
        return 0;
    }

    if (task->next_tick > scheduler->current_tick)
    {
        return 0;
    }

    return 1;
}

static int NxResearchScheduler_FindIndex(
    const NxResearchScheduler* scheduler,
    unsigned int task_id,
    size_t* index_out)
{
    size_t index = 0;

    if (scheduler == 0 || index_out == 0)
    {
        return 0;
    }

    for (index = 0; index < scheduler->count; ++index)
    {
        if (scheduler->tasks[index].id == task_id)
        {
            *index_out = index;
            return 1;
        }
    }

    return 0;
}

NxResearchSchedulerStatus NxResearchScheduler_Init(
    NxResearchScheduler* scheduler,
    size_t capacity)
{
    if (scheduler == 0 || capacity == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    scheduler->tasks = (NxResearchTaskDescriptor*)calloc(
        capacity,
        sizeof(NxResearchTaskDescriptor));

    if (scheduler->tasks == 0)
    {
        scheduler->count = 0;
        scheduler->capacity = 0;
        scheduler->current_tick = 0;
        return NX_RESEARCH_SCHEDULER_STATUS_OUT_OF_MEMORY;
    }

    scheduler->count = 0;
    scheduler->capacity = capacity;
    scheduler->current_tick = 0;

    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

void NxResearchScheduler_Destroy(NxResearchScheduler* scheduler)
{
    if (scheduler == 0)
    {
        return;
    }

    free(scheduler->tasks);
    scheduler->tasks = 0;
    scheduler->count = 0;
    scheduler->capacity = 0;
    scheduler->current_tick = 0;
}

NxResearchSchedulerStatus NxResearchScheduler_Add(
    NxResearchScheduler* scheduler,
    const NxResearchTaskDescriptor* task)
{
    NxResearchTaskDescriptor normalized;

    if (scheduler == 0 || scheduler->tasks == 0 || task == 0 || task->name == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    if (scheduler->count >= scheduler->capacity)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_CAPACITY_EXCEEDED;
    }

    normalized = *task;

    if (normalized.state != NX_RESEARCH_TASK_STATE_DISABLED &&
        normalized.state != NX_RESEARCH_TASK_STATE_COMPLETED &&
        normalized.state != NX_RESEARCH_TASK_STATE_RUNNING)
    {
        normalized.state = NX_RESEARCH_TASK_STATE_PENDING;
    }

    scheduler->tasks[scheduler->count] = normalized;
    scheduler->count += 1;

    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

size_t NxResearchScheduler_Count(const NxResearchScheduler* scheduler)
{
    if (scheduler == 0)
    {
        return 0;
    }

    return scheduler->count;
}

NxResearchSchedulerStatus NxResearchScheduler_Get(
    const NxResearchScheduler* scheduler,
    size_t index,
    NxResearchTaskDescriptor* task_out)
{
    if (scheduler == 0 || scheduler->tasks == 0 || task_out == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    if (index >= scheduler->count)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_NOT_FOUND;
    }

    *task_out = scheduler->tasks[index];
    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

NxResearchSchedulerStatus NxResearchScheduler_SelectNext(
    NxResearchScheduler* scheduler,
    NxResearchTaskDescriptor* task_out)
{
    size_t index = 0;
    size_t best_index = 0;
    int found = 0;

    if (scheduler == 0 || scheduler->tasks == 0 || task_out == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    for (index = 0; index < scheduler->count; ++index)
    {
        if (!NxResearchScheduler_IsEligible(scheduler, &scheduler->tasks[index]))
        {
            continue;
        }

        if (!found || scheduler->tasks[index].priority > scheduler->tasks[best_index].priority)
        {
            best_index = index;
            found = 1;
        }
    }

    if (!found)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_NO_ELIGIBLE_TASK;
    }

    scheduler->tasks[best_index].state = NX_RESEARCH_TASK_STATE_RUNNING;
    *task_out = scheduler->tasks[best_index];

    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

NxResearchSchedulerStatus NxResearchScheduler_MarkCompleted(
    NxResearchScheduler* scheduler,
    unsigned int task_id,
    unsigned int cooldown_ticks)
{
    size_t index = 0;

    if (scheduler == 0 || scheduler->tasks == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    if (!NxResearchScheduler_FindIndex(scheduler, task_id, &index))
    {
        return NX_RESEARCH_SCHEDULER_STATUS_NOT_FOUND;
    }

    scheduler->tasks[index].completed_runs += 1;

    if (scheduler->tasks[index].max_runs != 0 &&
        scheduler->tasks[index].completed_runs >= scheduler->tasks[index].max_runs)
    {
        scheduler->tasks[index].state = NX_RESEARCH_TASK_STATE_COMPLETED;
    }
    else
    {
        scheduler->tasks[index].state = NX_RESEARCH_TASK_STATE_PENDING;
        scheduler->tasks[index].next_tick = scheduler->current_tick + cooldown_ticks;
    }

    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

NxResearchSchedulerStatus NxResearchScheduler_Disable(
    NxResearchScheduler* scheduler,
    unsigned int task_id)
{
    size_t index = 0;

    if (scheduler == 0 || scheduler->tasks == 0)
    {
        return NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT;
    }

    if (!NxResearchScheduler_FindIndex(scheduler, task_id, &index))
    {
        return NX_RESEARCH_SCHEDULER_STATUS_NOT_FOUND;
    }

    scheduler->tasks[index].state = NX_RESEARCH_TASK_STATE_DISABLED;
    return NX_RESEARCH_SCHEDULER_STATUS_OK;
}

void NxResearchScheduler_AdvanceTick(
    NxResearchScheduler* scheduler,
    unsigned int ticks)
{
    if (scheduler == 0)
    {
        return;
    }

    scheduler->current_tick += ticks;
}
