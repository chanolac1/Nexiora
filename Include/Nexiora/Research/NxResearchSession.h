#ifndef NEXIORA_RESEARCH_NX_RESEARCH_SESSION_H
#define NEXIORA_RESEARCH_NX_RESEARCH_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResearchSessionStatus
{
    NX_RESEARCH_SESSION_STATUS_OK = 0,
    NX_RESEARCH_SESSION_STATUS_INVALID_ARGUMENT = 1
} NxResearchSessionStatus;

typedef struct NxResearchSession
{
    unsigned int id;
    unsigned int cycles_requested;
    unsigned int cycles_completed;
    unsigned int experiments_executed;
    unsigned int experiments_succeeded;
    unsigned int experiments_failed;
    unsigned int experiments_skipped;
} NxResearchSession;

NxResearchSessionStatus NxResearchSession_Begin(
    NxResearchSession* session,
    unsigned int id,
    unsigned int cycles_requested);

void NxResearchSession_RecordSuccess(NxResearchSession* session);
void NxResearchSession_RecordFailure(NxResearchSession* session);
void NxResearchSession_RecordSkipped(NxResearchSession* session);
void NxResearchSession_RecordCycle(NxResearchSession* session);

#ifdef __cplusplus
}
#endif

#endif
