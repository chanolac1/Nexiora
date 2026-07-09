#include "Nexiora/NCOS/NxPlanningEngine.h"

#include <stdio.h>
#include <string.h>

static int fails = 0;

static void expect(int cond, const char* msg)
{
    if (!cond) {
        printf("FAIL: %s\n", msg);
        fails++;
    }
}

int main(void)
{
    const char* root = ".nexiora_test_planning_engine_repair2";

    NxPlanCreateResult created;
    expect(NxPlanning_Create(root, "Crear un videojuego simple", &created) == 1, "create should succeed");
    expect(strcmp(created.id, "crear_un_videojuego_simple") == 0, "plan id should be normalized");
    expect(created.step_count == 5, "plan should create default steps");
    expect(created.path[0] != '\0', "plan should expose path");

    NxPlanStatus status;
    expect(NxPlanning_Status(root, &status) == 1, "status should find active plan");
    expect(status.exists == 1, "status should exist");
    expect(status.step_count == 5, "status should read steps");
    expect(status.completed_count == 0, "no steps should be completed initially");
    expect(strstr(status.goal, "videojuego") != NULL, "goal should be preserved");

    expect(NxPlanning_AddNote(root, "Definir loop de juego", &status) == 1, "note should succeed");
    expect(NxPlanning_CompleteStep(root, 1, &status) == 1, "complete step should succeed");
    expect(status.completed_count == 1, "completed step should be reflected in status");

    return fails == 0 ? 0 : 1;
}
