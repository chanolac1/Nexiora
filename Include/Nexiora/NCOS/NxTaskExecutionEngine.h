#ifndef NEXIORA_NCOS_NX_TASK_EXECUTION_ENGINE_H
#define NEXIORA_NCOS_NX_TASK_EXECUTION_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxTaskStatusTag
{
    NX_TASK_PENDING = 0,
    NX_TASK_READY,
    NX_TASK_RUNNING,
    NX_TASK_BLOCKED,
    NX_TASK_FAILED,
    NX_TASK_COMPLETED,
    NX_TASK_SKIPPED
} NxTaskStatus;

typedef struct NxTaskRecordTag
{
    int id;
    NxTaskStatus status;
    char title[192];
    char result[256];
} NxTaskRecord;

typedef struct NxTaskPlanSummaryTag
{
    char plan_id[96];
    char goal[256];
    char path[512];
    int total;
    int pending;
    int running;
    int completed;
    int failed;
    int blocked;
} NxTaskPlanSummary;

const char* NxTaskStatus_ToString(NxTaskStatus status);
int NxTaskStatus_FromString(const char* text, NxTaskStatus* out_status);

int NxTaskEngine_CreatePlan(const char* root,
                            const char* plan_id,
                            const char* goal,
                            const char* const* steps,
                            size_t step_count,
                            NxTaskPlanSummary* out_summary);

int NxTaskEngine_Status(const char* root,
                        const char* plan_id,
                        NxTaskPlanSummary* out_summary);

int NxTaskEngine_RunNext(const char* root,
                         const char* plan_id,
                         NxTaskRecord* out_task,
                         NxTaskPlanSummary* out_summary);

int NxTaskEngine_RunAll(const char* root,
                        const char* plan_id,
                        NxTaskPlanSummary* out_summary);

int NxTaskEngine_AppendLog(const char* root,
                           const char* plan_id,
                           const char* event_type,
                           const char* detail);

int NxTaskEngine_LogPath(const char* root,
                         const char* plan_id,
                         char* dst,
                         size_t dst_size);

#ifdef __cplusplus
}
#endif

#endif
