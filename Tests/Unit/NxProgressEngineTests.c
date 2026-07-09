#include "Nexiora/Research/NxProgressEngine.h"

#include <stdio.h>
#include <string.h>

static int g_event_count = 0;

static void OnProgressEvent(const NxProgressTask* task, const char* event_name, void* user_data)
{
    (void)user_data;
    if (task == 0 || event_name == 0)
    {
        return;
    }
    if (strcmp(event_name, "EVENT_TEST") == 0)
    {
        ++g_event_count;
    }
}

int main(void)
{
    NxProgressTask task;
    char bar[64];

    NxProgressTask_Init(&task, "INV-TEST", "Investigacion de prueba");
    if (task.status != NX_PROGRESS_STATUS_PENDING)
    {
        printf("Expected pending status\n");
        return 1;
    }

    NxProgressTask_Begin(&task, "Preparando plan");
    if (task.status != NX_PROGRESS_STATUS_RUNNING)
    {
        printf("Expected running status\n");
        return 1;
    }

    NxProgressTask_Update(&task, 42.0, "Descargando fuentes", "Fuente simulada");
    if (task.progress < 41.9 || task.progress > 42.1)
    {
        printf("Expected progress 42\n");
        return 1;
    }

    NxProgressTask_FormatBar(&task, bar, sizeof(bar));
    if (strstr(bar, "42%") == 0)
    {
        printf("Expected formatted progress bar, got: %s\n", bar);
        return 1;
    }

    NxProgressTask_Emit(&task, "EVENT_TEST", OnProgressEvent, 0);
    if (g_event_count != 1)
    {
        printf("Expected one event\n");
        return 1;
    }

    NxProgressTask_Finish(&task, "Investigacion completada");
    if (task.status != NX_PROGRESS_STATUS_OK || task.progress != 100.0)
    {
        printf("Expected completed task\n");
        return 1;
    }

    return 0;
}
