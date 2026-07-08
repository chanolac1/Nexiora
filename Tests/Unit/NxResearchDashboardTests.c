#include "Nexiora/Research/NxAutonomousExecution.h"
#include "Nexiora/Research/NxResearchDashboard.h"

#include <stdio.h>

int main(void)
{
    NxAutonomousExecutionResult execution;
    NxAutonomousExecutionStatus execution_status;
    NxResearchDashboardResult dashboard;
    NxResearchDashboardStatus dashboard_status;

    execution_status = NxAutonomousExecution_RunDefault(".", &execution);
    if (execution_status != NX_AUTONOMOUS_EXECUTION_OK)
    {
        printf("Expected autonomous execution to succeed\n");
        return 1;
    }

    dashboard_status = NxResearchDashboard_Generate(execution.session_path, &dashboard);
    if (dashboard_status != NX_RESEARCH_DASHBOARD_OK)
    {
        printf("Expected dashboard generation to succeed\n");
        return 1;
    }

    if (!dashboard.has_summary || !dashboard.has_report || !dashboard.has_metrics)
    {
        printf("Expected core report artifacts to be detected\n");
        return 1;
    }

    if (!dashboard.has_knowledge || !dashboard.has_graph_json)
    {
        printf("Expected knowledge artifacts to be detected\n");
        return 1;
    }

    if (!dashboard.has_graph_dot || !dashboard.has_graph_svg)
    {
        printf("Expected graph exports to be detected\n");
        return 1;
    }

    if (dashboard.dashboard_path[0] == '\0')
    {
        printf("Expected dashboard path\n");
        return 1;
    }

    return 0;
}
