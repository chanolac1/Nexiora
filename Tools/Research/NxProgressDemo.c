#include "Nexiora/Research/NxProgressEngine.h"

#include <stdio.h>

static void print_task(const NxProgressTask* task)
{
    char bar[80];
    NxProgressTask_FormatBar(task, bar, sizeof(bar));

    printf("%s\n", bar);
    printf("Estado          : %s\n", NxProgressStatus_ToString(task->status));
    printf("Paso actual     : %s\n", task->current_step);
    printf("Ultima actividad: %s\n\n", task->last_activity);
}

int main(int argc, char** argv)
{
    const char* topic = argc > 1 ? argv[1] : "SQLite";
    NxProgressTask task;

    printf("Nexiora - Progreso de investigacion\n");
    printf("Objetivo: %s\n\n", topic);

    NxProgressTask_Init(&task, "INV-0010-DEMO", "Investigacion observable");
    NxProgressTask_Begin(&task, "Preparando plan");
    print_task(&task);

    NxProgressTask_Update(&task, 15.0, "Buscando fuentes", "Seleccionando proveedores iniciales");
    print_task(&task);

    NxProgressTask_Update(&task, 35.0, "Clasificando fuentes", "Priorizando documentacion oficial");
    print_task(&task);

    NxProgressTask_Update(&task, 55.0, "Descargando documentacion", "Simulando fuente oficial");
    print_task(&task);

    NxProgressTask_Update(&task, 72.0, "Extrayendo conceptos", "Conceptos detectados: arquitectura, almacenamiento, transacciones");
    print_task(&task);

    NxProgressTask_Update(&task, 88.0, "Actualizando memoria", "Preparando conocimiento persistente");
    print_task(&task);

    NxProgressTask_Finish(&task, "Investigacion completada; reporte listo");
    print_task(&task);

    printf("Resultado:\n");
    printf("  La investigacion observable termino correctamente.\n");
    printf("  Este comando aun no descarga Internet real; valida el framework de progreso.\n");

    return 0;
}
