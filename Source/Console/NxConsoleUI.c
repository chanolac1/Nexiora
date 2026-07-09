#include "Nexiora/Console/NxConsoleUI.h"

#include <stdio.h>
#include <string.h>

static void nx_console_clear_and_home(FILE* stream)
{
    fputs("\x1b[2J\x1b[H", stream);
}

static void nx_console_render_bar(double progress, char* buffer, size_t buffer_size)
{
    const int width = 28;
    int filled;
    int i;
    size_t pos = 0;

    if (buffer == 0 || buffer_size == 0)
    {
        return;
    }

    if (progress < 0.0) progress = 0.0;
    if (progress > 100.0) progress = 100.0;

    filled = (int)((progress / 100.0) * (double)width + 0.5);
    if (filled < 0) filled = 0;
    if (filled > width) filled = width;

    if (pos + 1 < buffer_size) buffer[pos++] = '[';
    for (i = 0; i < width && pos + 1 < buffer_size; ++i)
    {
        buffer[pos++] = (i < filled) ? '#' : '.';
    }
    if (pos + 1 < buffer_size) buffer[pos++] = ']';
    buffer[pos] = '\0';
}

static const char* nx_console_spinner(unsigned int frame)
{
    static const char* frames[] = { "|", "/", "-", "\\" };
    return frames[frame % 4U];
}

void NxConsoleUI_Init(NxConsoleUI* ui, FILE* stream, int interactive)
{
    if (ui == 0)
    {
        return;
    }

    ui->stream = stream != 0 ? stream : stdout;
    ui->interactive = interactive;
    ui->frame_started = 0;
    ui->frame_count = 0U;
}

void NxConsoleUI_Begin(NxConsoleUI* ui)
{
    if (ui == 0 || ui->stream == 0)
    {
        return;
    }

    if (ui->interactive)
    {
        fputs("\x1b[?25l", ui->stream);
        nx_console_clear_and_home(ui->stream);
    }

    ui->frame_started = 1;
    fflush(ui->stream);
}

void NxConsoleUI_RenderProgress(
    NxConsoleUI* ui,
    const NxProgressTask* task,
    const char* objective,
    const char* source,
    size_t sources_done,
    size_t sources_total,
    size_t concepts,
    size_t relations,
    const char* activity_line)
{
    char bar[64];
    FILE* out;
    const char* status;

    if (ui == 0 || task == 0)
    {
        return;
    }

    out = ui->stream != 0 ? ui->stream : stdout;
    status = NxProgressStatus_ToString(task->status);
    nx_console_render_bar(task->progress, bar, sizeof(bar));

    if (!ui->frame_started)
    {
        NxConsoleUI_Begin(ui);
    }

    if (ui->interactive)
    {
        fputs("\x1b[H", out);
    }

    fprintf(out, "============================================================\n");
    fprintf(out, "                 N E X I O R A                              \n");
    fprintf(out, "          Investigacion observable en vivo                  \n");
    fprintf(out, "============================================================\n\n");
    fprintf(out, "Investigacion : %-42s\n", task->id);
    fprintf(out, "Objetivo      : %-42s\n", objective != 0 ? objective : "-");
    fprintf(out, "Estado        : %-42s\n", status != 0 ? status : "-");
    fprintf(out, "Paso          : %-42s\n", task->current_step);
    fprintf(out, "Actividad     : %-42s\n", task->last_activity);
    fprintf(out, "Fuente actual : %-42s\n", source != 0 ? source : "-");
    fprintf(out, "\n");
    fprintf(out, "Progreso      : %s %5.1f %%  %s\n", bar, task->progress, nx_console_spinner(ui->frame_count));
    fprintf(out, "Fuentes       : %zu / %zu\n", sources_done, sources_total);
    fprintf(out, "Conceptos     : %zu\n", concepts);
    fprintf(out, "Relaciones    : %zu\n", relations);
    fprintf(out, "\n");
    fprintf(out, "Ultimo evento : %-42s\n", activity_line != 0 ? activity_line : "-");
    fprintf(out, "\n");
    fprintf(out, "Nota          : la pantalla se redibuja; no deberia desplazarse.\n");
    fprintf(out, "============================================================\n");

    ui->frame_count++;
    fflush(out);
}

void NxConsoleUI_End(NxConsoleUI* ui)
{
    if (ui == 0 || ui->stream == 0)
    {
        return;
    }

    if (ui->interactive)
    {
        fputs("\x1b[?25h", ui->stream);
    }

    fflush(ui->stream);
}
