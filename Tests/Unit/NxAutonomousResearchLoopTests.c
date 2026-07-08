#include "Nexiora/Research/NxAutonomousResearchLoop.h"

#include <stdio.h>
#include <string.h>

typedef struct LoopContext
{
    unsigned int executed;
    unsigned int evidence;
    unsigned int journal;
    unsigned int graph;
} LoopContext;

static int ExecuteExperiment(const NxResearchQueueItem* item, void* user_data)
{
    LoopContext* context = (LoopContext*)user_data;
    if (item == 0 || context == 0)
    {
        return 1;
    }
    context->executed += 1;
    return strcmp(item->experiment_id, "failing-experiment") == 0 ? 1 : 0;
}

static int CollectEvidence(const NxResearchQueueItem* item, void* user_data)
{
    LoopContext* context = (LoopContext*)user_data;
    (void)item;
    context->evidence += 1;
    return 0;
}

static int WriteJournal(const NxResearchSession* session, const NxResearchQueueItem* item, void* user_data)
{
    LoopContext* context = (LoopContext*)user_data;
    (void)session;
    (void)item;
    context->journal += 1;
    return 0;
}

static int UpdateGraph(const NxResearchSession* session, const NxResearchQueueItem* item, void* user_data)
{
    LoopContext* context = (LoopContext*)user_data;
    (void)session;
    (void)item;
    context->graph += 1;
    return 0;
}

int main(void)
{
    NxResearchQueue queue;
    NxResearchQueueItem storage[8];
    NxResearchPolicy policy;
    NxAutonomousResearchLoopCallbacks callbacks;
    NxAutonomousResearchLoopResult result;
    LoopContext context;

    if (NxResearchQueue_Init(&queue, storage, 8) != NX_RESEARCH_QUEUE_STATUS_OK)
    {
        printf("queue init failed\n");
        return 1;
    }

    (void)NxResearchQueue_Enqueue(&queue, "candidate-a", 100, 1, 0);
    (void)NxResearchQueue_Enqueue(&queue, "failing-experiment", 80, 1, 0);
    (void)NxResearchQueue_Enqueue(&queue, "candidate-b", 70, 1, 0);

    NxResearchPolicy_Default(&policy);
    policy.minimum_priority = 0;
    policy.max_items_per_session = 3;

    context.executed = 0;
    context.evidence = 0;
    context.journal = 0;
    context.graph = 0;

    callbacks.execute_experiment = ExecuteExperiment;
    callbacks.collect_evidence = CollectEvidence;
    callbacks.write_journal = WriteJournal;
    callbacks.update_graph = UpdateGraph;

    if (NxAutonomousResearchLoop_Run(&queue, &policy, &callbacks, &context, 3, &result) !=
        NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_OK)
    {
        printf("loop run failed\n");
        return 1;
    }

    if (result.cycles_completed != 3 || result.experiments_executed != 3 ||
        result.experiments_succeeded != 2 || result.experiments_failed != 1)
    {
        printf("loop result mismatch\n");
        return 1;
    }

    if (context.executed != 3 || context.evidence != 2 || context.journal != 2 || context.graph != 2)
    {
        printf("callback counters mismatch\n");
        return 1;
    }

    return 0;
}
