#ifndef NEXIORA_RESEARCH_NX_AUTONOMOUS_EXECUTION_H
#define NEXIORA_RESEARCH_NX_AUTONOMOUS_EXECUTION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_AUTONOMOUS_EXECUTION_PATH_MAX 260
#define NX_AUTONOMOUS_EXECUTION_MESSAGE_MAX 256

typedef enum NxAutonomousExecutionStatus
{
    NX_AUTONOMOUS_EXECUTION_OK = 0,
    NX_AUTONOMOUS_EXECUTION_INVALID_ARGUMENT = 1,
    NX_AUTONOMOUS_EXECUTION_IO_ERROR = 2,
    NX_AUTONOMOUS_EXECUTION_DISCOVERY_FAILED = 3,
    NX_AUTONOMOUS_EXECUTION_SCHEDULER_FAILED = 4,
    NX_AUTONOMOUS_EXECUTION_RUNNER_FAILED = 5,
    NX_AUTONOMOUS_EXECUTION_EVIDENCE_FAILED = 6,
    NX_AUTONOMOUS_EXECUTION_PROMOTION_FAILED = 7,
    NX_AUTONOMOUS_EXECUTION_GRAPH_FAILED = 8
} NxAutonomousExecutionStatus;

typedef struct NxAutonomousExecutionResult
{
    char session_path[NX_AUTONOMOUS_EXECUTION_PATH_MAX];
    size_t experiments_discovered;
    size_t experiments_executed;
    size_t evidence_generated;
    size_t promotion_candidates;
    size_t graph_nodes;
    size_t graph_edges;
    char message[NX_AUTONOMOUS_EXECUTION_MESSAGE_MAX];
} NxAutonomousExecutionResult;

const char* NxAutonomousExecution_StatusToString(
    NxAutonomousExecutionStatus status);

NxAutonomousExecutionStatus NxAutonomousExecution_RunDefault(
    const char* workspace_root,
    NxAutonomousExecutionResult* result_out);

#ifdef __cplusplus
}
#endif

#endif
