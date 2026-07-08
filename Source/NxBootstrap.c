#include "Nexiora/NCP/Logging/NxLog.h"
#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/Research/NxAutonomousExecution.h"
#include "Nexiora/Research/NxResearchDashboard.h"

#include <stdio.h>
#include <string.h>

static void nx_print_research_run_result(const NxAutonomousExecutionResult* result)
{
    printf("------------------------------------------------\n");
    printf(" Nexiora Autonomous Research Session\n");
    printf("------------------------------------------------\n\n");
    printf("Session Path          : %s\n", result->session_path);
    printf("Experiments Discovered: %zu\n", result->experiments_discovered);
    printf("Experiments Executed  : %zu\n", result->experiments_executed);
    printf("Evidence Generated    : %zu\n", result->evidence_generated);
    printf("Promotion Candidates  : %zu\n", result->promotion_candidates);
    printf("Knowledge Graph Nodes : %zu\n", result->graph_nodes);
    printf("Knowledge Graph Edges : %zu\n\n", result->graph_edges);
    printf("Recommendation Gate   : Waiting for human approval\n");
    printf("Runtime Promotion     : Not automatic\n\n");
    printf("Artifacts:\n");
    printf("  summary.txt\n");
    printf("  report.md\n");
    printf("  metrics.json\n");
    printf("  knowledge.json\n");
    printf("  graph.json\n");
    printf("  graph.dot\n");
    printf("  graph.svg\n");
    printf("  dashboard.html\n\n");
    printf("%s\n", result->message);
}

static void nx_print_dashboard_result(const NxResearchDashboardResult* result)
{
    printf("------------------------------------------------\n");
    printf(" Nexiora Research Dashboard\n");
    printf("------------------------------------------------\n\n");
    printf("Dashboard Path : %s\n\n", result->dashboard_path);
    printf("Artifacts detected:\n");
    printf("  summary.txt             : %s\n", result->has_summary ? "yes" : "no");
    printf("  report.md               : %s\n", result->has_report ? "yes" : "no");
    printf("  metrics.json            : %s\n", result->has_metrics ? "yes" : "no");
    printf("  knowledge.json          : %s\n", result->has_knowledge ? "yes" : "no");
    printf("  graph.json              : %s\n", result->has_graph_json ? "yes" : "no");
    printf("  graph.dot               : %s\n", result->has_graph_dot ? "yes" : "no");
    printf("  graph.svg               : %s\n\n", result->has_graph_svg ? "yes" : "no");
    printf("%s\n", result->message);
}

int main(int argc, char** argv)
{
    NxRuntime runtime;
    NxResult result;

    result = nx_runtime_initialize(&runtime, "nexiora.log");
    if (result != NX_OK)
    {
        fprintf(stderr, "Failed to initialize Nexiora: %s\n", nx_result_to_string(result));
        return 1;
    }

    if (argc >= 3 && strcmp(argv[1], "research") == 0 && strcmp(argv[2], "run") == 0)
    {
        NxAutonomousExecutionResult run_result;
        NxAutonomousExecutionStatus run_status;
        NxResearchDashboardResult dashboard_result;

        nx_log_write(NX_LOG_INFO, "Research", "Starting first autonomous research execution.");
        run_status = NxAutonomousExecution_RunDefault(".", &run_result);
        if (run_status != NX_AUTONOMOUS_EXECUTION_OK)
        {
            fprintf(stderr,
                "Autonomous research execution failed: %s\n",
                NxAutonomousExecution_StatusToString(run_status));
            nx_runtime_shutdown(&runtime);
            return 2;
        }

        (void)NxResearchDashboard_Generate(run_result.session_path, &dashboard_result);
        nx_print_research_run_result(&run_result);
        printf("Dashboard generated : %s\n", dashboard_result.dashboard_path);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    if (argc >= 3 && strcmp(argv[1], "research") == 0 && strcmp(argv[2], "dashboard") == 0)
    {
        NxResearchDashboardResult dashboard_result;
        NxResearchDashboardStatus dashboard_status;

        nx_log_write(NX_LOG_INFO, "Research", "Generating autonomous research dashboard.");
        dashboard_status = NxResearchDashboard_GenerateDefault(".", &dashboard_result);
        if (dashboard_status != NX_RESEARCH_DASHBOARD_OK)
        {
            fprintf(stderr,
                "Research dashboard generation failed: %s\n",
                NxResearchDashboard_StatusToString(dashboard_status));
            nx_runtime_shutdown(&runtime);
            return 3;
        }

        nx_print_dashboard_result(&dashboard_result);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    nx_log_write(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis.");
    nx_runtime_print_status(&runtime);
    printf("\nCommands:\n");
    printf("  nexiora research run          Execute the first autonomous research session\n");
    printf("  nexiora research dashboard    Generate dashboard.html for the latest session\n");

    nx_runtime_shutdown(&runtime);
    return 0;
}
