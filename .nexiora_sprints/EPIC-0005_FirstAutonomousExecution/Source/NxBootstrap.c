#include "Nexiora/NCP/Logging/NxLog.h"
#include "Nexiora/NCP/Runtime/NxRuntime.h"
#include "Nexiora/Research/NxAutonomousExecution.h"

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
    printf("  graph.svg\n\n");
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

        nx_print_research_run_result(&run_result);
        nx_runtime_shutdown(&runtime);
        return 0;
    }

    nx_log_write(NX_LOG_INFO, "Bootstrap", "Welcome to Nexiora Genesis.");
    nx_runtime_print_status(&runtime);
    printf("\nCommands:\n");
    printf("  nexiora research run    Execute the first autonomous research session\n");

    nx_runtime_shutdown(&runtime);
    return 0;
}
