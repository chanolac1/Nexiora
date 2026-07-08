#include "Nexiora/Research/NxAutonomousResearchLoop.h"

static void NxAutonomousResearchLoop_CopySessionToResult(
    const NxResearchSession* session,
    NxAutonomousResearchLoopResult* result)
{
    if (session == 0 || result == 0)
    {
        return;
    }
    result->cycles_completed = session->cycles_completed;
    result->experiments_executed = session->experiments_executed;
    result->experiments_succeeded = session->experiments_succeeded;
    result->experiments_failed = session->experiments_failed;
    result->experiments_skipped = session->experiments_skipped;
}

NxAutonomousResearchLoopStatus NxAutonomousResearchLoop_Run(
    NxResearchQueue* queue,
    const NxResearchPolicy* policy,
    const NxAutonomousResearchLoopCallbacks* callbacks,
    void* user_data,
    unsigned int requested_cycles,
    NxAutonomousResearchLoopResult* result_out)
{
    NxResearchSession session;
    unsigned int cycle;

    if (queue == 0 || callbacks == 0 || result_out == 0 || callbacks->execute_experiment == 0)
    {
        return NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_INVALID_ARGUMENT;
    }

    result_out->cycles_completed = 0;
    result_out->experiments_executed = 0;
    result_out->experiments_succeeded = 0;
    result_out->experiments_failed = 0;
    result_out->experiments_skipped = 0;

    if (NxResearchSession_Begin(&session, 1, requested_cycles) != NX_RESEARCH_SESSION_STATUS_OK)
    {
        return NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_INVALID_ARGUMENT;
    }

    for (cycle = 0; cycle < requested_cycles; ++cycle)
    {
        NxResearchQueueItem item;
        NxResearchQueueStatus queue_status;
        NxResearchPolicyDecision decision;
        int callback_status;

        queue_status = NxResearchQueue_SelectNext(queue, &item);
        if (queue_status == NX_RESEARCH_QUEUE_STATUS_EMPTY)
        {
            NxAutonomousResearchLoop_CopySessionToResult(&session, result_out);
            return session.experiments_executed == 0 && session.experiments_skipped == 0
                ? NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_QUEUE_EMPTY
                : NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_OK;
        }
        if (queue_status != NX_RESEARCH_QUEUE_STATUS_OK)
        {
            NxAutonomousResearchLoop_CopySessionToResult(&session, result_out);
            return NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_INVALID_ARGUMENT;
        }

        decision = NxResearchPolicy_Evaluate(policy, &item, session.experiments_executed);
        if (decision == NX_RESEARCH_POLICY_DECISION_STOP)
        {
            (void)NxResearchQueue_MarkSkipped(queue, item.id);
            NxResearchSession_RecordSkipped(&session);
            NxAutonomousResearchLoop_CopySessionToResult(&session, result_out);
            return NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_OK;
        }
        if (decision == NX_RESEARCH_POLICY_DECISION_SKIP)
        {
            (void)NxResearchQueue_MarkSkipped(queue, item.id);
            NxResearchSession_RecordSkipped(&session);
            NxResearchSession_RecordCycle(&session);
            continue;
        }

        callback_status = callbacks->execute_experiment(&item, user_data);
        if (callback_status != 0)
        {
            (void)NxResearchQueue_MarkFailed(queue, item.id);
            NxResearchSession_RecordFailure(&session);
            NxResearchSession_RecordCycle(&session);
            continue;
        }

        if (callbacks->collect_evidence != 0 && callbacks->collect_evidence(&item, user_data) != 0)
        {
            (void)NxResearchQueue_MarkFailed(queue, item.id);
            NxResearchSession_RecordFailure(&session);
            NxResearchSession_RecordCycle(&session);
            continue;
        }

        if (callbacks->write_journal != 0 && callbacks->write_journal(&session, &item, user_data) != 0)
        {
            (void)NxResearchQueue_MarkFailed(queue, item.id);
            NxResearchSession_RecordFailure(&session);
            NxResearchSession_RecordCycle(&session);
            continue;
        }

        if (callbacks->update_graph != 0 && callbacks->update_graph(&session, &item, user_data) != 0)
        {
            (void)NxResearchQueue_MarkFailed(queue, item.id);
            NxResearchSession_RecordFailure(&session);
            NxResearchSession_RecordCycle(&session);
            continue;
        }

        (void)NxResearchQueue_MarkCompleted(queue, item.id);
        NxResearchSession_RecordSuccess(&session);
        NxResearchSession_RecordCycle(&session);
    }

    NxAutonomousResearchLoop_CopySessionToResult(&session, result_out);
    return NX_AUTONOMOUS_RESEARCH_LOOP_STATUS_OK;
}
