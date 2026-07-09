#include "Nexiora/NCOS/NxTaskExecutionEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

static void check(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    const char* root = ".nexiora_test_ncos008";
    const char* plan = "Juego Simple Test 008";
    const char* steps[] = {
        "Crear estructura del proyecto",
        "Generar CMake",
        "Escribir loop principal"
    };
    NxTaskPlanSummary s;
    NxTaskRecord task;
    char log_path[512];
    FILE* f;

#ifdef _WIN32
    system("if exist .nexiora_test_ncos008 rmdir /s /q .nexiora_test_ncos008 >nul 2>nul");
#else
    system("rm -rf .nexiora_test_ncos008 >/dev/null 2>/dev/null");
#endif

    check(NxTaskEngine_CreatePlan(root, plan, "Crear un videojuego simple", steps, 3, &s), "create should succeed");
    check(strcmp(s.plan_id, "juego_simple_test_008") == 0, "plan id should be normalized");
    check(s.total == 3, "plan should contain three tasks");
    check(s.pending == 3, "created plan should have ready/pending tasks");
    check(strstr(s.path, "juego_simple_test_008.tasks") != NULL, "summary should expose task path");

    check(NxTaskEngine_RunNext(root, plan, &task, &s), "run next should succeed");
    check(task.id == 1, "first task should run");
    check(task.status == NX_TASK_COMPLETED, "first task should complete");
    check(s.completed == 1, "summary should count completed task");

    check(NxTaskEngine_RunAll(root, plan, &s), "run all should succeed");
    check(s.completed == 3, "all tasks should complete");
    check(s.failed == 0, "deterministic executor should not fail");

    check(NxTaskEngine_LogPath(root, plan, log_path, sizeof(log_path)), "log path should be produced");
    f = fopen(log_path, "r");
    check(f != NULL, "action log should exist");
    if (f != NULL) fclose(f);

#ifdef _WIN32
    system("if exist .nexiora_test_ncos008 rmdir /s /q .nexiora_test_ncos008 >nul 2>nul");
#else
    system("rm -rf .nexiora_test_ncos008 >/dev/null 2>/dev/null");
#endif

    return failures == 0 ? 0 : 1;
}
