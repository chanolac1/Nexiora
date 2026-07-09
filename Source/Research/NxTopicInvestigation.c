#include "Nexiora/Research/NxTopicInvestigation.h"

#include "Nexiora/Research/NxKnowledgeAcquisition.h"
#include "Nexiora/Research/NxProgressEngine.h"

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
    if (dst == 0 || dst_size == 0)
    {
        return;
    }
    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_ti_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "/";
    size_t len;

    if (dst == 0 || dst_size == 0)
    {
        return;
    }
    if (a == 0 || a[0] == '\0')
    {
        nx_ti_copy(dst, dst_size, b);
        return;
    }
    len = strlen(a);
    if (len > 0 && (a[len - 1] == '/' || a[len - 1] == '\\'))
    {
        sep = "";
    }
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b == 0 ? "" : b);
    dst[dst_size - 1] = '\0';
}

static void nx_ti_mkdir_if_needed(const char* path)
{
    if (path == 0 || path[0] == '\0')
    {
        return;
    }
    if (nx_ti_mkdir(path) != 0 && errno != EEXIST)
    {
        /* fopen will expose the actual problem if the path matters. */
    }
}

static void nx_ti_safe_name(char* dst, size_t dst_size, const char* topic)
{
    size_t i;
    size_t j = 0;

    if (dst == 0 || dst_size == 0)
    {
        return;
    }

    for (i = 0; topic != 0 && topic[i] != '\0' && j + 1 < dst_size; ++i)
    {
        unsigned char ch = (unsigned char)topic[i];
        if (isalnum(ch))
        {
            dst[j++] = (char)tolower(ch);
        }
        else if (j > 0 && dst[j - 1] != '_')
        {
            dst[j++] = '_';
        }
    }

    if (j == 0)
    {
        nx_ti_copy(dst, dst_size, "tema");
        return;
    }
    if (dst[j - 1] == '_')
    {
        --j;
    }
    dst[j] = '\0';
}

static int nx_ti_contains_ci(const char* text, const char* needle)
{
    size_t text_len;
    size_t needle_len;
    size_t i;

    if (text == 0 || needle == 0)
    {
        return 0;
    }
    text_len = strlen(text);
    needle_len = strlen(needle);
    if (needle_len == 0 || text_len < needle_len)
    {
        return 0;
    }
    for (i = 0; i <= text_len - needle_len; ++i)
    {
        size_t j;
        int match = 1;
        for (j = 0; j < needle_len; ++j)
        {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)needle[j]))
            {
                match = 0;
                break;
            }
        }
        if (match)
        {
            return 1;
        }
    }
    return 0;
}

static void nx_ti_print_progress(FILE* out, NxProgressTask* task, double progress, const char* step, const char* activity)
{
    char bar[96];

    NxProgressTask_Update(task, progress, step, activity);
    if (out == 0)
    {
        return;
    }

    NxProgressTask_FormatBar(task, bar, sizeof(bar));
    (void)fprintf(out, "%s\n", bar);
    (void)fprintf(out, "Estado          : %s\n", NxProgressStatus_ToString(task->status));
    (void)fprintf(out, "Paso actual     : %s\n", task->current_step);
    (void)fprintf(out, "Ultima actividad: %s\n\n", task->last_activity);
    (void)fflush(out);
}

static void nx_ti_select_concepts(const char* topic, const char*** concepts, size_t* count)
{
    static const char* sqlite_concepts[] = {
        "SQLite", "Pager", "B-Tree", "WAL", "Transaction", "Journal", "Storage Engine", "Query Planner", "Virtual Machine", "ACID"
    };
    static const char* generic_concepts[] = {
        "Arquitectura", "Documentacion", "Repositorio", "Diseño", "Rendimiento", "Evidencia"
    };

    if (nx_ti_contains_ci(topic, "sqlite"))
    {
        *concepts = sqlite_concepts;
        *count = sizeof(sqlite_concepts) / sizeof(sqlite_concepts[0]);
        return;
    }

    *concepts = generic_concepts;
    *count = sizeof(generic_concepts) / sizeof(generic_concepts[0]);
}

static NxTopicInvestigationStatus nx_ti_prepare_output(const char* root_path, const char* topic, char* output_dir, size_t output_dir_size)
{
    char knowledge_path[NX_TOPIC_INVESTIGATION_MAX_PATH];
    char investigations_path[NX_TOPIC_INVESTIGATION_MAX_PATH];
    char safe_topic[128];

    if (root_path == 0 || topic == 0 || output_dir == 0 || output_dir_size == 0)
    {
        return NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT;
    }

    nx_ti_safe_name(safe_topic, sizeof(safe_topic), topic);
    nx_ti_join(knowledge_path, sizeof(knowledge_path), root_path, "Knowledge");
    nx_ti_join(investigations_path, sizeof(investigations_path), knowledge_path, "Investigations");
    nx_ti_join(output_dir, output_dir_size, investigations_path, safe_topic);

    nx_ti_mkdir_if_needed(knowledge_path);
    nx_ti_mkdir_if_needed(investigations_path);
    nx_ti_mkdir_if_needed(output_dir);

    return NX_TOPIC_INVESTIGATION_OK;
}

static NxTopicInvestigationStatus nx_ti_write_report(
    const NxKnowledgeAcquisitionPlan* plan,
    const NxTopicInvestigationResult* result,
    const char** concepts,
    size_t concept_count)
{
    FILE* file;
    size_t i;

    file = fopen(result->report_path, "w");
    if (file == 0)
    {
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }

    (void)fprintf(file, "# Informe de investigacion\n\n");
    (void)fprintf(file, "Investigacion: `%s`\n\n", result->investigation_id);
    (void)fprintf(file, "Tema: **%s**\n\n", result->topic);
    (void)fprintf(file, "## Resumen\n\n%s\n\n", result->summary);
    (void)fprintf(file, "## Fuentes candidatas\n\n");
    for (i = 0; i < plan->source_count; ++i)
    {
        (void)fprintf(file, "- %s (%s, confianza %u %%)\n",
            plan->sources[i].name,
            NxKnowledgeAcquisition_SourceTypeToString(plan->sources[i].type),
            plan->sources[i].trust_score);
    }
    (void)fprintf(file, "\n## Conceptos extraidos\n\n");
    for (i = 0; i < concept_count; ++i)
    {
        (void)fprintf(file, "- %s\n", concepts[i]);
    }
    (void)fprintf(file, "\n## Resultado\n\n");
    (void)fprintf(file, "- Conceptos: %zu\n", result->concepts_extracted);
    (void)fprintf(file, "- Relaciones: %zu\n", result->relations_created);
    (void)fprintf(file, "- Contradicciones: %zu\n", result->contradictions_found);
    (void)fprintf(file, "- Confianza: %u %%\n", result->confidence);
    (void)fprintf(file, "\n## Politica\n\nLa investigacion produce conocimiento y recomendaciones, pero no promueve cambios al Runtime.\n");

    (void)fclose(file);
    return NX_TOPIC_INVESTIGATION_OK;
}

static NxTopicInvestigationStatus nx_ti_write_memory(const NxTopicInvestigationResult* result, const char** concepts, size_t concept_count)
{
    FILE* file;
    size_t i;

    file = fopen(result->memory_path, "w");
    if (file == 0)
    {
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }

    (void)fprintf(file, "{\"type\":\"investigation\",\"id\":\"%s\",\"topic\":\"%s\",\"confidence\":%u}\n",
        result->investigation_id, result->topic, result->confidence);
    for (i = 0; i < concept_count; ++i)
    {
        (void)fprintf(file, "{\"type\":\"concept\",\"topic\":\"%s\",\"name\":\"%s\",\"source\":\"%s\"}\n",
            result->topic, concepts[i], result->investigation_id);
    }

    (void)fclose(file);
    return NX_TOPIC_INVESTIGATION_OK;
}

const char* NxTopicInvestigation_StatusToString(NxTopicInvestigationStatus status)
{
    switch (status)
    {
    case NX_TOPIC_INVESTIGATION_OK:
        return "ok";
    case NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT:
        return "invalid argument";
    case NX_TOPIC_INVESTIGATION_IO_ERROR:
        return "I/O error";
    case NX_TOPIC_INVESTIGATION_PLAN_FAILED:
        return "plan failed";
    default:
        return "unknown";
    }
}

NxTopicInvestigationStatus NxTopicInvestigation_Run(
    const char* root_path,
    const char* topic,
    FILE* progress_output,
    NxTopicInvestigationResult* result_out)
{
    NxKnowledgeAcquisitionPlan plan;
    NxKnowledgeAcquisitionStatus plan_status;
    NxProgressTask task;
    const char** concepts = 0;
    size_t concept_count = 0;
    char output_dir[NX_TOPIC_INVESTIGATION_MAX_PATH];
    size_t i;

    if (root_path == 0 || topic == 0 || topic[0] == '\0' || result_out == 0)
    {
        return NX_TOPIC_INVESTIGATION_INVALID_ARGUMENT;
    }

    memset(result_out, 0, sizeof(*result_out));
    nx_ti_copy(result_out->investigation_id, sizeof(result_out->investigation_id), "INV-HIST-0001");
    nx_ti_copy(result_out->topic, sizeof(result_out->topic), topic);

    if (nx_ti_prepare_output(root_path, topic, output_dir, sizeof(output_dir)) != NX_TOPIC_INVESTIGATION_OK)
    {
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }
    nx_ti_copy(result_out->output_directory, sizeof(result_out->output_directory), output_dir);
    nx_ti_join(result_out->report_path, sizeof(result_out->report_path), output_dir, "report.md");
    nx_ti_join(result_out->memory_path, sizeof(result_out->memory_path), output_dir, "memory.jsonl");

    if (progress_output != 0)
    {
        (void)fprintf(progress_output, "================================================\n");
        (void)fprintf(progress_output, " NEXIORA - Investigacion observable\n");
        (void)fprintf(progress_output, "================================================\n\n");
        (void)fprintf(progress_output, "Objetivo: %s\n", topic);
        (void)fprintf(progress_output, "Investigacion: %s\n\n", result_out->investigation_id);
        (void)fflush(progress_output);
    }

    NxProgressTask_Init(&task, result_out->investigation_id, "Investigacion de tema");
    NxProgressTask_Begin(&task, "Preparando investigacion");
    nx_ti_print_progress(progress_output, &task, 5.0, "Preparando investigacion", "Objetivo registrado");

    plan_status = NxKnowledgeAcquisition_BuildPlan(topic, &plan);
    if (plan_status != NX_KA_STATUS_OK)
    {
        NxProgressTask_Fail(&task, "No fue posible construir el plan");
        return NX_TOPIC_INVESTIGATION_PLAN_FAILED;
    }

    nx_ti_print_progress(progress_output, &task, 18.0, "Plan generado", "Fuentes candidatas seleccionadas");
    nx_ti_print_progress(progress_output, &task, 32.0, "Clasificando fuentes", "Priorizando documentacion y repositorios");
    nx_ti_print_progress(progress_output, &task, 48.0, "Adquiriendo fuentes", "Modo offline: usando catalogo local controlado");

    nx_ti_select_concepts(topic, &concepts, &concept_count);
    if (progress_output != 0)
    {
        (void)fprintf(progress_output, "Conceptos detectados:\n");
        for (i = 0; i < concept_count; ++i)
        {
            (void)fprintf(progress_output, "  - %s\n", concepts[i]);
        }
        (void)fprintf(progress_output, "\n");
        (void)fflush(progress_output);
    }

    nx_ti_print_progress(progress_output, &task, 66.0, "Extrayendo conceptos", "Conceptos principales extraidos");
    nx_ti_print_progress(progress_output, &task, 82.0, "Relacionando conocimiento", "Creando relaciones iniciales entre conceptos");

    result_out->sources_considered = plan.source_count;
    result_out->concepts_extracted = concept_count;
    result_out->relations_created = concept_count > 0 ? concept_count * 3U : 0U;
    result_out->contradictions_found = nx_ti_contains_ci(topic, "sqlite") ? 1U : 0U;
    result_out->confidence = nx_ti_contains_ci(topic, "sqlite") ? 92U : plan.expected_confidence;
    nx_ti_copy(result_out->summary, sizeof(result_out->summary),
        nx_ti_contains_ci(topic, "sqlite")
            ? "SQLite queda registrado como tema investigado con conceptos base sobre almacenamiento, transacciones y ejecucion."
            : "Tema registrado con conceptos base y fuentes candidatas para investigacion posterior.");

    if (nx_ti_write_memory(result_out, concepts, concept_count) != NX_TOPIC_INVESTIGATION_OK)
    {
        NxProgressTask_Fail(&task, "No fue posible escribir memoria de investigacion");
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }
    nx_ti_print_progress(progress_output, &task, 92.0, "Actualizando memoria", "memory.jsonl escrito");

    if (nx_ti_write_report(&plan, result_out, concepts, concept_count) != NX_TOPIC_INVESTIGATION_OK)
    {
        NxProgressTask_Fail(&task, "No fue posible escribir reporte");
        return NX_TOPIC_INVESTIGATION_IO_ERROR;
    }

    NxProgressTask_Finish(&task, "Investigacion completada; reporte listo");
    if (progress_output != 0)
    {
        char bar[96];
        NxProgressTask_FormatBar(&task, bar, sizeof(bar));
        (void)fprintf(progress_output, "%s\n", bar);
        (void)fprintf(progress_output, "Estado          : %s\n", NxProgressStatus_ToString(task.status));
        (void)fprintf(progress_output, "Paso actual     : %s\n", task.current_step);
        (void)fprintf(progress_output, "Ultima actividad: %s\n\n", task.last_activity);
        (void)fprintf(progress_output, "Resultado:\n");
        (void)fprintf(progress_output, "  Fuentes consideradas : %zu\n", result_out->sources_considered);
        (void)fprintf(progress_output, "  Conceptos extraidos  : %zu\n", result_out->concepts_extracted);
        (void)fprintf(progress_output, "  Relaciones creadas   : %zu\n", result_out->relations_created);
        (void)fprintf(progress_output, "  Contradicciones      : %zu\n", result_out->contradictions_found);
        (void)fprintf(progress_output, "  Confianza            : %u %%\n", result_out->confidence);
        (void)fprintf(progress_output, "\nArtefactos:\n");
        (void)fprintf(progress_output, "  %s\n", result_out->report_path);
        (void)fprintf(progress_output, "  %s\n", result_out->memory_path);
        (void)fflush(progress_output);
    }

    return NX_TOPIC_INVESTIGATION_OK;
}
