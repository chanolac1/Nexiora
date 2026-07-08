#include "Nexiora/Research/NxResearchSession.h"

#include <string.h>

NxResearchSessionStatus NxResearchSession_Begin(
    NxResearchSession* session,
    unsigned int id,
    unsigned int cycles_requested)
{
    if (session == 0)
    {
        return NX_RESEARCH_SESSION_STATUS_INVALID_ARGUMENT;
    }
    memset(session, 0, sizeof(*session));
    session->id = id;
    session->cycles_requested = cycles_requested;
    return NX_RESEARCH_SESSION_STATUS_OK;
}

void NxResearchSession_RecordSuccess(NxResearchSession* session)
{
    if (session == 0)
    {
        return;
    }
    session->experiments_executed += 1;
    session->experiments_succeeded += 1;
}

void NxResearchSession_RecordFailure(NxResearchSession* session)
{
    if (session == 0)
    {
        return;
    }
    session->experiments_executed += 1;
    session->experiments_failed += 1;
}

void NxResearchSession_RecordSkipped(NxResearchSession* session)
{
    if (session == 0)
    {
        return;
    }
    session->experiments_skipped += 1;
}

void NxResearchSession_RecordCycle(NxResearchSession* session)
{
    if (session == 0)
    {
        return;
    }
    session->cycles_completed += 1;
}
