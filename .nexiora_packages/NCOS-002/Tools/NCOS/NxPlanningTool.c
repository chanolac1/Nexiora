#include "Nexiora/NCOS/NxPlanningEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_status(const NxPlanStatus* s)
{
    if (s == NULL || !s->exists) {
        printf("No hay plan activo.\n");
        return;
    }
    printf("Plan activo\n");
    printf("ID       : %s\n", s->id);
    printf("Objetivo : %s\n", s->goal);
    printf("Archivo  : %s\n", s->path);
    printf("Progreso : %d/%d pasos completados\n", s->completed_count, s->step_count);
    printf("\nPasos:\n");
    for (int i = 0; i < s->step_count; ++i) {
        printf("  [%s] %d. %s\n", strcmp(s->steps[i].status, "done") == 0 ? "x" : " ", s->steps[i].index, s->steps[i].title);
    }
}

int main(int argc, char** argv)
{
    const char* root = ".";
    if (argc < 2) {
        printf("Uso:\n");
        printf("  nexiora_plan create \"objetivo\"\n");
        printf("  nexiora_plan status\n");
        printf("  nexiora_plan done <paso>\n");
        printf("  nexiora_plan note \"texto\"\n");
        return 0;
    }

    if (strcmp(argv[1], "create") == 0) {
        if (argc < 3) {
            printf("Falta objetivo.\n");
            return 1;
        }
        NxPlanCreateResult r;
        if (!NxPlanning_Create(root, argv[2], &r)) {
            printf("No se pudo crear el plan.\n");
            return 2;
        }
        printf("Plan creado.\n");
        printf("ID       : %s\n", r.id);
        printf("Objetivo : %s\n", r.goal);
        printf("Pasos    : %d\n", r.step_count);
        printf("Archivo  : %s\n", r.path);
        return 0;
    }

    if (strcmp(argv[1], "status") == 0) {
        NxPlanStatus s;
        if (!NxPlanning_Status(root, &s)) {
            printf("No hay plan activo.\n");
            return 1;
        }
        print_status(&s);
        return 0;
    }

    if (strcmp(argv[1], "done") == 0) {
        if (argc < 3) {
            printf("Falta numero de paso.\n");
            return 1;
        }
        NxPlanStatus s;
        if (!NxPlanning_CompleteStep(root, atoi(argv[2]), &s)) {
            printf("No se pudo actualizar el plan.\n");
            return 2;
        }
        printf("Paso registrado como completado.\n");
        print_status(&s);
        return 0;
    }

    if (strcmp(argv[1], "note") == 0) {
        if (argc < 3) {
            printf("Falta nota.\n");
            return 1;
        }
        NxPlanStatus s;
        if (!NxPlanning_AddNote(root, argv[2], &s)) {
            printf("No se pudo registrar la nota.\n");
            return 2;
        }
        printf("Nota registrada.\n");
        print_status(&s);
        return 0;
    }

    printf("Comando no reconocido: %s\n", argv[1]);
    return 1;
}
