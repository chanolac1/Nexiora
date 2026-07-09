#ifndef NEXIORA_RESEARCH_NX_PROGRESS_ENGINE_H
#define NEXIORA_RESEARCH_NX_PROGRESS_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum NxProgressStatus
{
    NX_PROGRESS_STATUS_PENDING = 0,
    NX_PROGRESS_STATUS_RUNNING = 1,
    NX_PROGRESS_STATUS_OK = 2,
    NX_PROGRESS_STATUS_WARNING = 3,
    NX_PROGRESS_STATUS_ERROR = 4,
    NX_PROGRESS_STATUS_CANCELLED = 5
} NxProgressStatus;

typedef struct NxProgressTask
{
    char id[64];
    char title[256];
    char current_step[256];
    char last_activity[256];
    double progress;
    unsigned long long started_at;
    unsigned long long updated_at;
    NxProgressStatus status;
} NxProgressTask;

typedef void (*NxProgressEventCallback)(const NxProgressTask* task, const char* event_name, void* user_data);

void NxProgressTask_Init(NxProgressTask* task, const char* id, const char* title);
void NxProgressTask_Begin(NxProgressTask* task, const char* step);
void NxProgressTask_Update(NxProgressTask* task, double progress, const char* step, const char* activity);
void NxProgressTask_Warn(NxProgressTask* task, const char* activity);
void NxProgressTask_Fail(NxProgressTask* task, const char* activity);
void NxProgressTask_Cancel(NxProgressTask* task, const char* activity);
void NxProgressTask_Finish(NxProgressTask* task, const char* activity);
const char* NxProgressStatus_ToString(NxProgressStatus status);
void NxProgressTask_FormatBar(const NxProgressTask* task, char* buffer, size_t buffer_size);
void NxProgressTask_Emit(const NxProgressTask* task, const char* event_name, NxProgressEventCallback callback, void* user_data);

#ifdef __cplusplus
}
#endif

#endif
