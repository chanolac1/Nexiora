#include "Nexiora/NCOS/NxIntentPlanner.h"
#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("Uso:\n");
    printf("  nexiora_intent_plan.exe \"crear un videojuego simple\"\n");
    printf("  nexiora_intent_plan.exe \"investiga GeneXus\"\n");
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage();
        return 1;
    }

    char request[512] = {0};
    for (int i = 1; i < argc; ++i) {
        if (i > 1) strncat(request, " ", sizeof(request) - strlen(request) - 1);
        strncat(request, argv[i], sizeof(request) - strlen(request) - 1);
    }

    NxIntentPlanResult result;
    if (!NxIntentPlanner_CreatePlan(".", request, &result)) {
        fprintf(stderr, "No fue posible crear el plan desde la intencion.\n");
        return 2;
    }

    printf("Plan creado desde intencion.\n");
    printf("Intencion : %s\n", result.intent);
    printf("Objetivo  : %s\n", result.normalized_goal);
    printf("Target    : %s\n", result.target);
    printf("Plan ID   : %s\n", result.plan.id);
    printf("Pasos     : %d\n", result.plan.step_count);
    printf("Archivo   : %s\n", result.plan.path);
    return 0;
}
