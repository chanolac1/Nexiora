#ifndef NEXIORA_CONSOLE_NX_CONSOLE_UI_H
#define NEXIORA_CONSOLE_NX_CONSOLE_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Nexiora/Research/NxProgressEngine.h"

#include <stddef.h>
#include <stdio.h>

typedef struct NxConsoleUI
{
    FILE* stream;
    int interactive;
    int frame_started;
    unsigned int frame_count;
} NxConsoleUI;

void NxConsoleUI_Init(NxConsoleUI* ui, FILE* stream, int interactive);
void NxConsoleUI_Begin(NxConsoleUI* ui);
void NxConsoleUI_RenderProgress(
    NxConsoleUI* ui,
    const NxProgressTask* task,
    const char* objective,
    const char* source,
    size_t sources_done,
    size_t sources_total,
    size_t concepts,
    size_t relations,
    const char* activity_line);
void NxConsoleUI_End(NxConsoleUI* ui);

#ifdef __cplusplus
}
#endif

#endif
