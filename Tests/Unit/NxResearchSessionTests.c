#include "Nexiora/Research/NxResearchSession.h"

#include <stdio.h>

int main(void)
{
    NxResearchSession session;

    if (NxResearchSession_Begin(&session, 7, 3) != NX_RESEARCH_SESSION_STATUS_OK)
    {
        printf("session begin failed\n");
        return 1;
    }

    NxResearchSession_RecordSuccess(&session);
    NxResearchSession_RecordFailure(&session);
    NxResearchSession_RecordSkipped(&session);
    NxResearchSession_RecordCycle(&session);

    if (session.id != 7 || session.cycles_requested != 3 || session.cycles_completed != 1)
    {
        printf("session identity or cycle mismatch\n");
        return 1;
    }

    if (session.experiments_executed != 2 || session.experiments_succeeded != 1 ||
        session.experiments_failed != 1 || session.experiments_skipped != 1)
    {
        printf("session counters mismatch\n");
        return 1;
    }

    return 0;
}
