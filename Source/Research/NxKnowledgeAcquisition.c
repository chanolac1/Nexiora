#include "Nexiora/Research/NxKnowledgeAcquisition.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define nx_ka_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define nx_ka_mkdir(path) mkdir((path), 0777)
#endif

static int nx_ka_contains_ci(const char* text, const char* needle)
{
    size_t text_len;
    size_t needle_len;
    size_t i;

    if (text == NULL || needle == NULL)
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

static void nx_ka_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0)
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_ka_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    if (dst == NULL || dst_size == 0)
    {
        return;
    }

    if (a == NULL || a[0] == '\0')
    {
        nx_ka_copy(dst, dst_size, b);
        return;
    }

    (void)snprintf(dst, dst_size, "%s/%s", a, b == NULL ? "" : b);
    dst[dst_size - 1] = '\0';
}

static void nx_ka_safe_name(char* dst, size_t dst_size, const char* topic)
{
    size_t i;
    size_t j = 0;

    if (dst == NULL || dst_size == 0)
    {
        return;
    }

    for (i = 0; topic != NULL && topic[i] != '\0' && j + 1 < dst_size; ++i)
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
        nx_ka_copy(dst, dst_size, "topic");
        return;
    }

    if (dst[j - 1] == '_')
    {
        --j;
    }
    dst[j] = '\0';
}

static void nx_ka_make_dir_if_needed(const char* path)
{
    if (path == NULL || path[0] == '\0')
    {
        return;
    }

    if (nx_ka_mkdir(path) != 0 && errno != EEXIST)
    {
        /* The final file open will report the real error if this matters. */
    }
}

static void nx_ka_add_source(
    NxKnowledgeAcquisitionPlan* plan,
    NxKnowledgeSourceType type,
    const char* name,
    const char* reason,
    unsigned int trust_score,
    unsigned int priority)
{
    NxKnowledgeSourceCandidate* source;

    if (plan == NULL || plan->source_count >= NX_KA_MAX_SOURCES)
    {
        return;
    }

    source = &plan->sources[plan->source_count++];
    source->type = type;
    nx_ka_copy(source->name, sizeof(source->name), name);
    nx_ka_copy(source->reason, sizeof(source->reason), reason);
    source->trust_score = trust_score;
    source->priority = priority;
}

static void nx_ka_add_step(NxKnowledgeAcquisitionPlan* plan, const char* step)
{
    if (plan == NULL || plan->step_count >= NX_KA_MAX_STEPS)
    {
        return;
    }

    nx_ka_copy(plan->steps[plan->step_count++], NX_KA_MAX_STEP_LENGTH, step);
}

const char* NxKnowledgeAcquisition_StatusToString(NxKnowledgeAcquisitionStatus status)
{
    switch (status)
    {
    case NX_KA_STATUS_OK:
        return "ok";
    case NX_KA_STATUS_INVALID_ARGUMENT:
        return "invalid argument";
    case NX_KA_STATUS_TOPIC_TOO_LONG:
        return "topic too long";
    case NX_KA_STATUS_IO_ERROR:
        return "I/O error";
    default:
        return "unknown";
    }
}

const char* NxKnowledgeAcquisition_SourceTypeToString(NxKnowledgeSourceType type)
{
    switch (type)
    {
    case NX_KNOWLEDGE_SOURCE_OFFICIAL_DOCUMENTATION:
        return "documentacion oficial";
    case NX_KNOWLEDGE_SOURCE_GITHUB_REPOSITORY:
        return "repositorio GitHub";
    case NX_KNOWLEDGE_SOURCE_RFC:
        return "RFC";
    case NX_KNOWLEDGE_SOURCE_RESEARCH_PAPER:
        return "paper";
    case NX_KNOWLEDGE_SOURCE_BOOK:
        return "libro";
    case NX_KNOWLEDGE_SOURCE_LOCAL_FILE:
        return "archivo local";
    case NX_KNOWLEDGE_SOURCE_TECHNICAL_ARTICLE:
        return "articulo tecnico";
    default:
        return "desconocido";
    }
}

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_BuildPlan(
    const char* topic,
    NxKnowledgeAcquisitionPlan* plan_out)
{
    int looks_like_book;
    int looks_like_rfc;
    int looks_like_github;
    int looks_like_file;

    if (topic == NULL || topic[0] == '\0' || plan_out == NULL)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    if (strlen(topic) >= NX_KA_MAX_TOPIC_LENGTH)
    {
        return NX_KA_STATUS_TOPIC_TOO_LONG;
    }

    memset(plan_out, 0, sizeof(*plan_out));
    nx_ka_copy(plan_out->topic, sizeof(plan_out->topic), topic);

    looks_like_book = nx_ka_contains_ci(topic, "libro") ||
        nx_ka_contains_ci(topic, "book") ||
        nx_ka_contains_ci(topic, "clean architecture") ||
        nx_ka_contains_ci(topic, "designing data");
    looks_like_rfc = nx_ka_contains_ci(topic, "rfc") || nx_ka_contains_ci(topic, "quic");
    looks_like_github = nx_ka_contains_ci(topic, "github") ||
        nx_ka_contains_ci(topic, "repositorio") ||
        nx_ka_contains_ci(topic, "sqlite") ||
        nx_ka_contains_ci(topic, "llvm") ||
        nx_ka_contains_ci(topic, "mimalloc");
    looks_like_file = nx_ka_contains_ci(topic, ".pdf") ||
        nx_ka_contains_ci(topic, ".md") ||
        nx_ka_contains_ci(topic, ".txt");

    if (looks_like_book)
    {
        nx_ka_copy(plan_out->kind, sizeof(plan_out->kind), "book");
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_BOOK, "Libro proporcionado o fuentes publicas del libro", "Usar solo contenido con permiso; si no existe archivo local, usar metadatos, reseñas y material publico.", 80, 1);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_OFFICIAL_DOCUMENTATION, "Sitio oficial del autor/editorial", "Fuente primaria para indice, errata, ejemplos y material complementario.", 90, 2);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_TECHNICAL_ARTICLE, "Analisis tecnicos y reseñas", "Ayuda a contrastar interpretaciones, sin reemplazar la fuente primaria.", 65, 3);
    }
    else if (looks_like_rfc)
    {
        nx_ka_copy(plan_out->kind, sizeof(plan_out->kind), "rfc");
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_RFC, "RFC oficial", "Fuente normativa principal para protocolos y estandares.", 98, 1);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_OFFICIAL_DOCUMENTATION, "Documentacion del grupo de trabajo", "Contexto historico y cambios de diseño.", 88, 2);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_GITHUB_REPOSITORY, "Implementaciones de referencia", "Permite estudiar decisiones practicas y compatibilidad.", 75, 3);
    }
    else if (looks_like_file)
    {
        nx_ka_copy(plan_out->kind, sizeof(plan_out->kind), "local-file");
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_LOCAL_FILE, "Archivo local proporcionado", "Fuente directa para extraer conceptos y generar notas.", 90, 1);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_TECHNICAL_ARTICLE, "Fuentes publicas relacionadas", "Sirven para contrastar el contenido local.", 65, 2);
    }
    else
    {
        nx_ka_copy(plan_out->kind, sizeof(plan_out->kind), looks_like_github ? "software" : "general");
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_OFFICIAL_DOCUMENTATION, "Documentacion oficial", "Punto de partida con mayor autoridad sobre el tema.", 92, 1);
        if (looks_like_github)
        {
            nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_GITHUB_REPOSITORY, "Repositorio oficial", "Permite estudiar arquitectura, modulos, pruebas y decisiones reales.", 86, 2);
        }
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_RESEARCH_PAPER, "Papers y publicaciones tecnicas", "Aporta fundamentos, evaluaciones y comparaciones.", 82, 3);
        nx_ka_add_source(plan_out, NX_KNOWLEDGE_SOURCE_TECHNICAL_ARTICLE, "Articulos tecnicos", "Ayuda a encontrar experiencia practica y casos de uso.", 68, 4);
    }

    nx_ka_add_step(plan_out, "Definir el objetivo y las preguntas que debe responder la investigacion.");
    nx_ka_add_step(plan_out, "Reunir fuentes candidatas y priorizarlas por confiabilidad.");
    nx_ka_add_step(plan_out, "Extraer conceptos, decisiones de diseño, restricciones y riesgos.");
    nx_ka_add_step(plan_out, "Buscar contradicciones entre fuentes y marcar confianza baja si existen.");
    nx_ka_add_step(plan_out, "Relacionar lo aprendido con Nexiora y su BOOK.");
    nx_ka_add_step(plan_out, "Generar reporte, conocimiento persistente e hipotesis de seguimiento.");

    plan_out->estimated_minutes = looks_like_book ? 45U : (looks_like_rfc ? 30U : 25U);
    plan_out->expected_confidence = looks_like_rfc ? 92U : (looks_like_book ? 78U : 84U);

    return NX_KA_STATUS_OK;
}

static NxKnowledgeAcquisitionStatus nx_ka_prepare_output(
    const char* root_path,
    const char* topic,
    char* dir_path,
    size_t dir_path_size)
{
    char research_path[NX_KA_MAX_PATH_LENGTH];
    char acquisition_path[NX_KA_MAX_PATH_LENGTH];
    char safe_topic[NX_KA_MAX_PATH_LENGTH];

    if (root_path == NULL || topic == NULL || dir_path == NULL || dir_path_size == 0)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    nx_ka_join(research_path, sizeof(research_path), root_path, "Research");
    nx_ka_join(acquisition_path, sizeof(acquisition_path), research_path, "Acquisition");
    nx_ka_safe_name(safe_topic, sizeof(safe_topic), topic);
    nx_ka_join(dir_path, dir_path_size, acquisition_path, safe_topic);

    nx_ka_make_dir_if_needed(research_path);
    nx_ka_make_dir_if_needed(acquisition_path);
    nx_ka_make_dir_if_needed(dir_path);

    return NX_KA_STATUS_OK;
}

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_WritePlanMarkdown(
    const NxKnowledgeAcquisitionPlan* plan,
    const char* root_path,
    char* output_path,
    size_t output_path_size)
{
    char dir_path[NX_KA_MAX_PATH_LENGTH];
    FILE* file;
    size_t i;

    if (plan == NULL || root_path == NULL || output_path == NULL || output_path_size == 0)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    if (nx_ka_prepare_output(root_path, plan->topic, dir_path, sizeof(dir_path)) != NX_KA_STATUS_OK)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    nx_ka_join(output_path, output_path_size, dir_path, "plan.md");
    file = fopen(output_path, "w");
    if (file == NULL)
    {
        return NX_KA_STATUS_IO_ERROR;
    }

    (void)fprintf(file, "# Plan de adquisicion de conocimiento\n\n");
    (void)fprintf(file, "## Objetivo\n\n%s\n\n", plan->topic);
    (void)fprintf(file, "Tipo: `%s`\n\n", plan->kind);
    (void)fprintf(file, "Tiempo estimado: %u minutos\n\n", plan->estimated_minutes);
    (void)fprintf(file, "Confianza esperada: %u %%\n\n", plan->expected_confidence);

    (void)fprintf(file, "## Fuentes candidatas\n\n");
    for (i = 0; i < plan->source_count; ++i)
    {
        (void)fprintf(file,
            "- **%s** (%s) — prioridad %u, confianza %u %%\n  - %s\n",
            plan->sources[i].name,
            NxKnowledgeAcquisition_SourceTypeToString(plan->sources[i].type),
            plan->sources[i].priority,
            plan->sources[i].trust_score,
            plan->sources[i].reason);
    }

    (void)fprintf(file, "\n## Pasos\n\n");
    for (i = 0; i < plan->step_count; ++i)
    {
        (void)fprintf(file, "%zu. %s\n", i + 1U, plan->steps[i]);
    }

    (void)fprintf(file,
        "\n## Politica\n\n"
        "Nexiora debe separar hechos, inferencias y opiniones. "
        "La investigacion no promueve cambios al Runtime; solo prepara evidencia y recomendaciones para revision humana.\n");

    (void)fclose(file);
    return NX_KA_STATUS_OK;
}

NxKnowledgeAcquisitionStatus NxKnowledgeAcquisition_WritePlanJson(
    const NxKnowledgeAcquisitionPlan* plan,
    const char* root_path,
    char* output_path,
    size_t output_path_size)
{
    char dir_path[NX_KA_MAX_PATH_LENGTH];
    FILE* file;
    size_t i;

    if (plan == NULL || root_path == NULL || output_path == NULL || output_path_size == 0)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    if (nx_ka_prepare_output(root_path, plan->topic, dir_path, sizeof(dir_path)) != NX_KA_STATUS_OK)
    {
        return NX_KA_STATUS_INVALID_ARGUMENT;
    }

    nx_ka_join(output_path, output_path_size, dir_path, "plan.json");
    file = fopen(output_path, "w");
    if (file == NULL)
    {
        return NX_KA_STATUS_IO_ERROR;
    }

    (void)fprintf(file, "{\n");
    (void)fprintf(file, "  \"topic\": \"%s\",\n", plan->topic);
    (void)fprintf(file, "  \"kind\": \"%s\",\n", plan->kind);
    (void)fprintf(file, "  \"estimated_minutes\": %u,\n", plan->estimated_minutes);
    (void)fprintf(file, "  \"expected_confidence\": %u,\n", plan->expected_confidence);
    (void)fprintf(file, "  \"sources\": [\n");
    for (i = 0; i < plan->source_count; ++i)
    {
        (void)fprintf(file,
            "    {\"type\": \"%s\", \"name\": \"%s\", \"trust\": %u, \"priority\": %u}%s\n",
            NxKnowledgeAcquisition_SourceTypeToString(plan->sources[i].type),
            plan->sources[i].name,
            plan->sources[i].trust_score,
            plan->sources[i].priority,
            (i + 1U < plan->source_count) ? "," : "");
    }
    (void)fprintf(file, "  ],\n");
    (void)fprintf(file, "  \"steps\": [\n");
    for (i = 0; i < plan->step_count; ++i)
    {
        (void)fprintf(file, "    \"%s\"%s\n", plan->steps[i], (i + 1U < plan->step_count) ? "," : "");
    }
    (void)fprintf(file, "  ]\n");
    (void)fprintf(file, "}\n");

    (void)fclose(file);
    return NX_KA_STATUS_OK;
}
