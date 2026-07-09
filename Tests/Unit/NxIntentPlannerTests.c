#include "Nexiora/NCOS/NxIntentPlanner.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static int failures = 0;
static void check(int cond, const char* msg)
{
    if (!cond) {
        printf("FAIL: %s\n", msg);
        failures++;
    }
}

int main(void)
{
    const char* root = ".nexiora_test_intent_planner";
    (void)NX_MKDIR(root);

    char intent[64];
    char target[128];
    check(NxIntentPlanner_Classify("crear un videojuego simple", intent, sizeof(intent), target, sizeof(target)) == 1, "classify should succeed");
    check(strcmp(intent, "build_project") == 0, "should detect build_project intent");
    check(strstr(target, "videojuego") != NULL, "target should include videojuego");

    NxIntentPlanResult result;
    check(NxIntentPlanner_CreatePlan(root, "crear un videojuego simple", &result) == 1, "create plan from intent should succeed");
    check(result.ok == 1, "result should be ok");
    check(strcmp(result.intent, "build_project") == 0, "result should keep intent");
    check(strstr(result.normalized_goal, "Construir proyecto") != NULL, "goal should be normalized to build project");
    check(result.plan.step_count >= 5, "plan should create default steps");
    check(result.plan.path[0] != '\0', "plan should expose path");

    NxPlanStatus status;
    check(NxPlanning_Status(root, &status) == 1, "status should find active plan");
    check(status.exists == 1, "active plan should exist");
    check(strstr(status.goal, "Construir proyecto") != NULL, "status should preserve normalized goal");

    return failures == 0 ? 0 : 1;
}
