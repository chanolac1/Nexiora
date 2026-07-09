#include "Nexiora/NCOS/NxTaskExecutionEngine.h"

#include <stdio.h>
#include <string.h>

static void print_summary(const NxTaskPlanSummary* s)
{
    if (s == NULL) return;
    printf("Plan       : %s\n", s->plan_id);
    printf("Objetivo   : %s\n", s->goal);
    printf("Total      : %d\n", s->total);
    printf("Pendientes : %d\n", s->pending);
    printf("En ejecucion: %d\n", s->running);
    printf("Completadas: %d\n", s->completed);
    printf("Fallidas   : %d\n", s->failed);
    printf("Bloqueadas : %d\n", s->blocked);
    printf("Archivo    : %s\n", s->path);
}

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_task create <plan> <objetivo>\n");
    printf("  nexiora_task status <plan>\n");
    printf("  nexiora_task run-next <plan>\n");
    printf("  nexiora_task run-all <plan>\n");
    printf("  nexiora_task log <plan>\n");
}

int main(int argc, char** argv)
{
    const char* root = ".";
    NxTaskPlanSummary summary;
    if (argc < 3)
    {
        usage();
        return 1;
    }
    if (strcmp(argv[1], "create") == 0)
    {
        const char* goal = (argc >= 4) ? argv[3] : argv[2];
        if (!NxTaskEngine_CreatePlan(root, argv[2], goal, NULL, 0, &summary))
        {
            fprintf(stderr, "No se pudo crear el plan.\n");
            return 2;
        }
        printf("Plan de ejecucion creado.\n");
        print_summary(&summary);
        return 0;
    }
    if (strcmp(argv[1], "status") == 0)
    {
        if (!NxTaskEngine_Status(root, argv[2], &summary))
        {
            fprintf(stderr, "No se encontro el plan.\n");
            return 2;
        }
        printf("Estado del plan.\n");
        print_summary(&summary);
        return 0;
    }
    if (strcmp(argv[1], "run-next") == 0)
    {
        NxTaskRecord task;
        if (!NxTaskEngine_RunNext(root, argv[2], &task, &summary))
        {
            fprintf(stderr, "No se pudo ejecutar la siguiente tarea.\n");
            return 2;
        }
        printf("Tarea ejecutada.\n");
        printf("TASK-%03d: %s [%s]\n", task.id, task.title, NxTaskStatus_ToString(task.status));
        print_summary(&summary);
        return 0;
    }
    if (strcmp(argv[1], "run-all") == 0)
    {
        if (!NxTaskEngine_RunAll(root, argv[2], &summary))
        {
            fprintf(stderr, "No se pudo ejecutar el plan.\n");
            return 2;
        }
        printf("Plan ejecutado.\n");
        print_summary(&summary);
        return 0;
    }
    if (strcmp(argv[1], "log") == 0)
    {
        char path[512];
        FILE* f;
        char line[512];
        if (!NxTaskEngine_LogPath(root, argv[2], path, sizeof(path))) return 2;
        f = fopen(path, "r");
        if (f == NULL)
        {
            fprintf(stderr, "No existe action log.\n");
            return 2;
        }
        while (fgets(line, sizeof(line), f) != NULL) fputs(line, stdout);
        fclose(f);
        return 0;
    }
    usage();
    return 1;
}
