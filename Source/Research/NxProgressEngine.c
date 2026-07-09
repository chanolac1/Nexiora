#include "Nexiora/Research/NxProgressEngine.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

static void nx_progress_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == 0 || dst_size == 0)
    {
        return;
    }

    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }

#if defined(_MSC_VER)
    (void)strncpy_s(dst, dst_size, src, _TRUNCATE);
#else
    (void)snprintf(dst, dst_size, "%s", src);
#endif
}

static unsigned long long nx_progress_now(void)
{
    return (unsigned long long)time(0);
}

static double nx_progress_clamp(double value)
{
    if (value < 0.0)
    {
        return 0.0;
    }
    if (value > 100.0)
    {
        return 100.0;
    }
    return value;
}

void NxProgressTask_Init(NxProgressTask* task, const char* id, const char* title)
{
    if (task == 0)
    {
        return;
    }

    memset(task, 0, sizeof(*task));
    nx_progress_copy(task->id, sizeof(task->id), id != 0 ? id : "TASK-UNKNOWN");
    nx_progress_copy(task->title, sizeof(task->title), title != 0 ? title : "Untitled task");
    nx_progress_copy(task->current_step, sizeof(task->current_step), "pendiente");
    nx_progress_copy(task->last_activity, sizeof(task->last_activity), "sin actividad");
    task->progress = 0.0;
    task->started_at = 0;
    task->updated_at = 0;
    task->status = NX_PROGRESS_STATUS_PENDING;
}

void NxProgressTask_Begin(NxProgressTask* task, const char* step)
{
    if (task == 0)
    {
        return;
    }

    task->started_at = nx_progress_now();
    task->updated_at = task->started_at;
    task->status = NX_PROGRESS_STATUS_RUNNING;
    task->progress = 0.0;
    nx_progress_copy(task->current_step, sizeof(task->current_step), step != 0 ? step : "iniciando");
    nx_progress_copy(task->last_activity, sizeof(task->last_activity), "tarea iniciada");
}

void NxProgressTask_Update(NxProgressTask* task, double progress, const char* step, const char* activity)
{
    if (task == 0)
    {
        return;
    }

    task->updated_at = nx_progress_now();
    task->status = NX_PROGRESS_STATUS_RUNNING;
    task->progress = nx_progress_clamp(progress);
    if (step != 0)
    {
        nx_progress_copy(task->current_step, sizeof(task->current_step), step);
    }
    if (activity != 0)
    {
        nx_progress_copy(task->last_activity, sizeof(task->last_activity), activity);
    }
}

void NxProgressTask_Warn(NxProgressTask* task, const char* activity)
{
    if (task == 0)
    {
        return;
    }

    task->updated_at = nx_progress_now();
    task->status = NX_PROGRESS_STATUS_WARNING;
    if (activity != 0)
    {
        nx_progress_copy(task->last_activity, sizeof(task->last_activity), activity);
    }
}

void NxProgressTask_Fail(NxProgressTask* task, const char* activity)
{
    if (task == 0)
    {
        return;
    }

    task->updated_at = nx_progress_now();
    task->status = NX_PROGRESS_STATUS_ERROR;
    if (activity != 0)
    {
        nx_progress_copy(task->last_activity, sizeof(task->last_activity), activity);
    }
}

void NxProgressTask_Cancel(NxProgressTask* task, const char* activity)
{
    if (task == 0)
    {
        return;
    }

    task->updated_at = nx_progress_now();
    task->status = NX_PROGRESS_STATUS_CANCELLED;
    if (activity != 0)
    {
        nx_progress_copy(task->last_activity, sizeof(task->last_activity), activity);
    }
}

void NxProgressTask_Finish(NxProgressTask* task, const char* activity)
{
    if (task == 0)
    {
        return;
    }

    task->updated_at = nx_progress_now();
    task->status = NX_PROGRESS_STATUS_OK;
    task->progress = 100.0;
    nx_progress_copy(task->current_step, sizeof(task->current_step), "completado");
    if (activity != 0)
    {
        nx_progress_copy(task->last_activity, sizeof(task->last_activity), activity);
    }
}

const char* NxProgressStatus_ToString(NxProgressStatus status)
{
    switch (status)
    {
        case NX_PROGRESS_STATUS_PENDING: return "PENDIENTE";
        case NX_PROGRESS_STATUS_RUNNING: return "EN PROCESO";
        case NX_PROGRESS_STATUS_OK: return "OK";
        case NX_PROGRESS_STATUS_WARNING: return "ADVERTENCIA";
        case NX_PROGRESS_STATUS_ERROR: return "ERROR";
        case NX_PROGRESS_STATUS_CANCELLED: return "CANCELADO";
        default: return "DESCONOCIDO";
    }
}

void NxProgressTask_FormatBar(const NxProgressTask* task, char* buffer, size_t buffer_size)
{
    int filled;
    int i;
    size_t used;

    if (buffer == 0 || buffer_size == 0)
    {
        return;
    }

    buffer[0] = '\0';
    if (task == 0)
    {
        return;
    }

    filled = (int)((task->progress / 100.0) * 20.0 + 0.5);
    if (filled < 0)
    {
        filled = 0;
    }
    if (filled > 20)
    {
        filled = 20;
    }

    used = 0;
    used += (size_t)snprintf(buffer + used, buffer_size > used ? buffer_size - used : 0, "[");
    for (i = 0; i < 20 && used < buffer_size; ++i)
    {
        used += (size_t)snprintf(buffer + used, buffer_size > used ? buffer_size - used : 0, "%c", i < filled ? '#' : '.');
    }
    if (used < buffer_size)
    {
        (void)snprintf(buffer + used, buffer_size - used, "] %3.0f%%", task->progress);
    }
}

void NxProgressTask_Emit(const NxProgressTask* task, const char* event_name, NxProgressEventCallback callback, void* user_data)
{
    if (task == 0 || callback == 0)
    {
        return;
    }

    callback(task, event_name != 0 ? event_name : "EVENT_PROGRESS", user_data);
}
