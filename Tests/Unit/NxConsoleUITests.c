#include "Nexiora/Console/NxConsoleUI.h"
#include "Nexiora/Research/NxProgressEngine.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    NxProgressTask task;
    char bar[64];

    NxProgressTask_Init(&task, "TEST-UI", "Console UI Test");
    NxProgressTask_Begin(&task, "Inicio");
    NxProgressTask_Update(&task, 50.0, "Mitad", "Renderizando progreso");
    NxProgressTask_FormatBar(&task, bar, sizeof(bar));

    if (strchr(bar, '#') == 0)
    {
        printf("Expected progress bar to contain filled cells.\n");
        return 1;
    }

    return 0;
}
