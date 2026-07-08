#ifndef NEXIORA_RESEARCH_NX_AUTONOMOUS_RESEARCH_LOOP_H
#define NEXIORA_RESEARCH_NX_AUTONOMOUS_RESEARCH_LOOP_H

#include "Nexiora/Research/NxResearchPolicy.h"
#include "Nexiora/Research/NxResearchQueue.h"
#include "Nexiora/Research/NxResearchSession.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxAutonomousResearchLoopStatus
{
    NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_OK = 0,
    NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_INVALID_ARGUMENT = 1,
    NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_QUEUE_EMPTY = 2,
    NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_CALLBACK_FAILED = 3
} NxAutonomousResearchLoopStatus;

typedef struct NxAutonomousResearchLoopCallbacks
{
    int (*execute_experiment)(const NxResearchQueueItem* item, void* user_data);
    int (*collect_evidence)(const NxResearchQueueItem* item, void* user_data);
    int (*write_journal)(const NxResearchSession* session, const NxResearchQueueItem* item, void* user_data);
    int (*update_graph)(const NxResearchSession* session, const NxResearchQueueItem* item, void* user_data);
} NxAutonomousResearchLoopCallbacks;

typedef struct NxAutonomousResearchLoopResult
{
    unsigned int cycles_completed;
    unsigned int experiments_executed;
    unsigned int experiments_succeeded;
    unsigned int experiments_failed;
    unsigned int experiments_skipped;
} NxAutonomousResearchLoopResult;

NxAutonomousResearchLoopStatus NxAutonomousResearchLoop_Run(
    NxResearchQueue* queue,
    const NxResearchPolicy* policy,
    const NxAutonomousResearchLoopCallbacks* callbacks,
    void* user_data,
    unsigned int requested_cycles,
    NxAutonomousResearchLoopResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
