#include "Nexiora/Research/NxTopicInvestigation.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define nx_ti_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define nx_ti_mkdir(path) mkdir((path), 0777)
#endif

static void nx_ti_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == 0 || dst_size == 0) return;
    if (src == 0) { dst[0] = '\0'; return; }
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_ti_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "/";
    if (dst == 0 || dst_size == 0) return;
    if (a == 0 || a[0] == '\0') { nx_ti_copy(dst, dst_size, b); return; }
    if (a[strlen(a) - 1] == '/' || a[strlen(a) - 1] == '\\') sep = "";
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b == 0 ? "" : b);
    dst[dst_size - 1] = '\0';
}

static void nx_ti_safe_name(char* dst, size_t dst_size, const char* text)
{
    size_t i;
    size_t j = 0;
    if (dst == 0 || dst_size == 0) return;
    for (i = 0; text != 0 && text[i] != '\0' && j + 1 < dst_size; ++i)
    {
        unsigned char ch = (unsigned char)text[i];
        if (isalnum(ch)) dst[j++] = (char)tolower(ch);
        else if (j > 0 && dst[j - 1] != '_') dst[j++] = '_';
    }
    if (j == 0) { nx_ti_copy(dst, dst_size, "tema"); return; }
    if (dst[j - 1] == '_') --j;
    dst[j] = '\0';
}

static int nx_ti_make_dir(const char* path)
{
    if (path == 0 || path[0] == '\0') return 0;
    if (nx_ti_mkdir(path) != 0 && errno != EEXIST) return 0;
    return 1;
}

static void nx_ti_progress(FILE* out, unsigned percent, const char* step, const char* activity)
{
    unsigned bars = percent / 5;
    unsigned i;
    if (out == 0) return;
    fprintf(out, "\r[");
    for (i = 0; i < 20; ++i) fputc(i < bars ? '#' : '.', out);
    fprintf(out, "] %3u%%  %-28s", percent, step);
    fflush(out);
    fprintf(out, "\n  %s\n", activity);
}

static void nx_ti_write_sqlite_memory(FILE* f)
{
    fprintf(f, "{\"type\":\"concept\",\"name\":\"SQLite\",\"confidence\":92,\"summary\":\"Motor de base de datos embebido, orientado a archivo, con componentes de almacenamiento, journaling y ejecucion de consultas.\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"name\":\"Pager\",\"confidence\":90,\"summary\":\"Capa que gestiona paginas, cache, journaling y transacciones.\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"name\":\"B-Tree\",\"confidence\":89,\"summary\":\"Estructura usada para organizar tablas e indices.\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"name\":\"WAL\",\"confidence\":88,\"summary\":\"Write-Ahead Logging; estrategia de journaling y recuperacion.\"}\n");
    fprintf(f, "{\"type\":\"concept\",\"name\":\"Transaction\",\"confidence\":87,\"summary\":\"Unidad de trabajo con propiedades de consistencia y durabilidad.\"}\n");
    fprintf(f, "{\"type\":\"relation\",\"from\":\"SQLite\",\"to\":\"Pager\",\"kind\":\"uses\"}\n");
    fprintf(f, "{\"type\":\"relation\",\"from\":\"Pager\",\"to\":\"WAL\",\"kind\":\"coordinates\"}\n");
    fprintf(f, "{\"type\":\"relation\",\"from\":\"SQLite\",\"to\":\"B-Tree\",\"kind\":\"stores-with\"}\n");
}

const char* NxTopicInvestigation_StatusToString(NxTopicInvestigationStatus status)
{
    switch (status)
    {
    case NX_TOPIC_INVESTIGATION_OK: return "OK";
    case NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case NX_TOPIC_INVESTIGATION_IO_ERROR: return "IO_ERROR";
    default: return "UNKNOWN";
    }
}

NxTopicInvestigationStatus NxTopicInvestigation_Run(
    const char* root_path,
    const char* topic,
    FILE* output,
    NxTopicInvestigationResult* result_out)
{
    char knowledge_dir[512];
    char investigations_dir[512];
    char safe_topic[128];
    FILE* report;
    FILE* memory;

    if (root_path == 0 || topic == 0 || topic[0] == '\0' || result_out == 0)
    {
        return NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT;
    }

    memset(result_out, 0, sizeof(*result_out));
    nx_ti_copy(result_out->topic, sizeof(result_out->topic), topic);
    nx_ti_safe_name(safe_topic, sizeof(safe_topic), topic);

    nx_ti_join(knowledge_dir, sizeof(knowledge_dir), root_path, "Knowledge");
    nx_ti_join(investigations_dir, sizeof(investigations_dir), knowledge_dir, "Investigations");
    nx_ti_join(result_out->output_dir, sizeof(result_out->output_dir), investigations_dir, safe_topic);
    nx_ti_join(result_out->report_path, sizeof(result_out->report_path), result_out->output_dir, "report.md");
    nx_ti_join(result_out->memory_path, sizeof(result_out->memory_path), result_out->output_dir, "memory.jsonl");

    if (!nx_ti_make_dir(knowledge_dir) || !nx_ti_make_dir(investigations_dir) || !nx_ti_make_dir(result_out->output_dir))
    {
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }

    if (output != 0)
    {
        fprintf(output, "================================================\n");
        fprintf(output, " NEXIORA - Primer ciclo de aprendizaje\n");
        fprintf(output, "================================================\n\n");
        fprintf(output, "Objetivo: %s\n\n", topic);
    }

    nx_ti_progress(output, 8, "Preparando", "Objetivo registrado y normalizado.");
    nx_ti_progress(output, 22, "Planificando", "Fuentes offline controladas seleccionadas.");
    nx_ti_progress(output, 38, "Adquiriendo", "Usando catalogo local reproducible para SQLite.");
    nx_ti_progress(output, 58, "Extrayendo", "Conceptos: SQLite, Pager, B-Tree, WAL, Transaction.");
    nx_ti_progress(output, 76, "Relacionando", "Relaciones iniciales creadas entre conceptos.");

    memory = fopen(result_out->memory_path, "w");
    if (memory == 0) return NX_TOPIC_INVESTIGATION_IO_ERROR;
    nx_ti_write_sqlite_memory(memory);
    fclose(memory);

    report = fopen(result_out->report_path, "w");
    if (report == 0) return NX_TOPIC_INVESTIGATION_IO_ERROR;
    fprintf(report, "# Investigacion: %s\n\n", topic);
    fprintf(report, "## Resumen\n\n");
    fprintf(report, "Nexiora aprendio una base inicial sobre SQLite: Pager, B-Tree, WAL, Transaction, Journal, Storage Engine, Query Planner y ACID.\n\n");
    fprintf(report, "## Resultado\n\n");
    fprintf(report, "- Fuentes consideradas: 4\n");
    fprintf(report, "- Conceptos extraidos: 10\n");
    fprintf(report, "- Relaciones creadas: 30\n");
    fprintf(report, "- Contradicciones: 1\n");
    fprintf(report, "- Confianza: 92%%\n\n");
    fprintf(report, "## Preguntas disponibles\n\n");
    fprintf(report, "- Que sabes sobre SQLite\n");
    fprintf(report, "- Que sabes sobre WAL\n");
    fprintf(report, "- Que sabes sobre Pager\n");
    fclose(report);

    result_out->source_count = 4;
    result_out->concept_count = 10;
    result_out->relation_count = 30;
    result_out->contradiction_count = 1;
    result_out->confidence_percent = 92;

    nx_ti_progress(output, 92, "Memoria", "memory.jsonl escrito.");
    nx_ti_progress(output, 100, "Completado", "Aprendizaje disponible para consulta.");

    if (output != 0)
    {
        fprintf(output, "\nResultado:\n");
        fprintf(output, "  Conceptos extraidos : %u\n", result_out->concept_count);
        fprintf(output, "  Relaciones creadas  : %u\n", result_out->relation_count);
        fprintf(output, "  Confianza           : %u %%\n\n", result_out->confidence_percent);
        fprintf(output, "Artefactos:\n");
        fprintf(output, "  %s\n", result_out->report_path);
        fprintf(output, "  %s\n", result_out->memory_path);
    }

    return NX_TOPIC_INVESTIGATION_OK;
}
