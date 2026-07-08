#include "Nexiora/Research/NxResearchScheduler.h"

#include <stdio.h>
#include <time.h>

int main(void)
{
    NxResearchScheduler scheduler;
    NxResearchTaskDescriptor task;
    NxResearchTaskDescriptor selected;
    clock_t begin;
    clock_t end;
    int i;
    int selected_count = 0;

    if (NxResearchScheduler_Init(&scheduler, 1024) != NX_RESEARCH_SCHEDULER_STATUS_OK)
    {
        return 1;
    }

    for (i = 0; i < 1024; ++i)
    {
        task.id = (unsigned int)(i + 1);
        task.name = "benchmark_experiment";
        task.priority = i % 100;
        task.max_runs = 1;
        task.completed_runs = 0;
        task.next_tick = 0;
        task.state = NX_RESEARCH_TASK_STATE_PENDING;
        (void)NxResearchScheduler_Add(&scheduler, &task);
    }

    begin = clock();

    while (NxResearchScheduler_SelectNext(&scheduler, &selected) == NX_RESEARCH_SCHEDULER_STATUS_OK)
    {
        (void)NxResearchScheduler_MarkCompleted(&scheduler, selected.id, 0);
        selected_count += 1;
    }

    end = clock();

    printf("NxResearchSchedulerBench: selected %d tasks in %.6f seconds\n",
        selected_count,
        (double)(end - begin) / (double)CLOCKS_PER_SEC);

    NxResearchScheduler_Destroy(&scheduler);
    return selected_count == 1024 ? 0 : 1;
}
