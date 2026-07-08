#include "Nexiora/Research/NxAutonomousExecution.h"

#include <stdio.h>
#include <string.h>

static int file_exists(const char* path)
{
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        return 0;
    }
    fclose(file);
    return 1;
}

static void join_path(char* out, size_t out_size, const char* a, const char* b)
{
#ifdef _WIN32
    (void)snprintf(out, out_size, "%s\\%s", a, b);
#else
    (void)snprintf(out, out_size, "%s/%s", a, b);
#endif
    out[out_size - 1] = '\0';
}

int main(void)
{
    NxAutonomousExecutionResult result;
    NxAutonomousExecutionStatus status;
    char path[512];

    status = NxAutonomousExecution_RunDefault(".", &result);
    if (status != NX_AUTONOMOUS_EXECUTION_OK)
    {
        printf("Expected OK, got %s\n", NxAutonomousExecution_StatusToString(status));
        return 1;
    }

    if (result.experiments_discovered != 3)
    {
        printf("Expected 3 experiments discovered, got %zu\n", result.experiments_discovered);
        return 1;
    }

    if (result.experiments_executed != 3)
    {
        printf("Expected 3 experiments executed, got %zu\n", result.experiments_executed);
        return 1;
    }

    if (result.evidence_generated != 3)
    {
        printf("Expected 3 evidence records, got %zu\n", result.evidence_generated);
        return 1;
    }

    if (result.graph_nodes < 7 || result.graph_edges < 6)
    {
        printf("Expected graph to be populated\n");
        return 1;
    }

    join_path(path, sizeof(path), result.session_path, "summary.txt");
    if (!file_exists(path))
    {
        printf("Expected summary file: %s\n", path);
        return 1;
    }

    join_path(path, sizeof(path), result.session_path, "report.md");
    if (!file_exists(path))
    {
        printf("Expected report file: %s\n", path);
        return 1;
    }

    join_path(path, sizeof(path), result.session_path, "metrics.json");
    if (!file_exists(path))
    {
        printf("Expected metrics file: %s\n", path);
        return 1;
    }

    join_path(path, sizeof(path), result.session_path, "graph.json");
    if (!file_exists(path))
    {
        printf("Expected graph json file: %s\n", path);
        return 1;
    }

    return 0;
}
