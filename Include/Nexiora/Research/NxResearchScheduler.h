#ifndef NEXIORA_RESEARCH_NX_RESEARCH_SCHEDULER_H
#define NEXIORA_RESEARCH_NX_RESEARCH_SCHEDULER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchSchedulerStatus
{
    NX_RESEARCH_SCHEDULER_STATUS_OK = 0,
    NX_RESEARCH_SCHEDULER_STATUS_INVALID_ARGUMENT = 1,
    NX_RESEARCH_SCHEDULER_STATUS_OUT_OF_MEMORY = 2,
    NX_RESEARCH_SCHEDULER_STATUS_CAPACITY_EXCEEDED = 3,
    NX_RESEARCH_SCHEDULER_STATUS_NOT_FOUND = 4,
    NX_RESEARCH_SCHEDULER_STATUS_NO_ELIGIBLE_TASK = 5
} NxResearchSchedulerStatus;

typedef enum NxResearchTaskState
{
    NX_RESEARCH_TASK_STATE_PENDING = 0,
    NX_RESEARCH_TASK_STATE_RUNNING = 1,
    NX_RESEARCH_TASK_STATE_COMPLETED = 2,
    NX_RESEARCH_TASK_STATE_DISABLED = 3
} NxResearchTaskState;

typedef struct NxResearchTaskDescriptor
{
    unsigned int id;
    const char* name;
    int priority;
    unsigned int max_runs;
    unsigned int completed_runs;
    unsigned int next_tick;
    NxResearchTaskState state;
} NxResearchTaskDescriptor;

typedef struct NxResearchScheduler
{
    NxResearchTaskDescriptor* tasks;
    size_t count;
    size_t capacity;
    unsigned int current_tick;
} NxResearchScheduler;

NxResearchSchedulerStatus NxResearchScheduler_Init(
    NxResearchScheduler* scheduler,
    size_t capacity);

void NxResearchScheduler_Destroy(NxResearchScheduler* scheduler);

NxResearchSchedulerStatus NxResearchScheduler_Add(
    NxResearchScheduler* scheduler,
    const NxResearchTaskDescriptor* task);

size_t NxResearchScheduler_Count(const NxResearchScheduler* scheduler);

NxResearchSchedulerStatus NxResearchScheduler_Get(
    const NxResearchScheduler* scheduler,
    size_t index,
    NxResearchTaskDescriptor* task_out);

NxResearchSchedulerStatus NxResearchScheduler_SelectNext(
    NxResearchScheduler* scheduler,
    NxResearchTaskDescriptor* task_out);

NxResearchSchedulerStatus NxResearchScheduler_MarkCompleted(
    NxResearchScheduler* scheduler,
    unsigned int task_id,
    unsigned int cooldown_ticks);

NxResearchSchedulerStatus NxResearchScheduler_Disable(
    NxResearchScheduler* scheduler,
    unsigned int task_id);

void NxResearchScheduler_AdvanceTick(
    NxResearchScheduler* scheduler,
    unsigned int ticks);

#ifdef __cplusplus
}
#endif

#endif
