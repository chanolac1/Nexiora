#include "Nexiora/Research/NxLearningCore.h"

#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_SEP "\\"
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#define NX_SEP "/"
#endif

static void nx_lc_copy_trimmed(char* dst, unsigned long dst_size, const char* src)
{
    const char* start;
    const char* end;
    unsigned long len;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (src == 0) return;
    start = src;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') start++;
    end = start + strlen(start);
    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\r' || *(end - 1) == '\n')) end--;
    len = (unsigned long)(end - start);
    if (len >= dst_size) len = dst_size - 1;
    memcpy(dst, start, len);
    dst[len] = '\0';
}

static void nx_lc_slugify(char* dst, unsigned long dst_size, const char* topic)
{
    unsigned long i;
    unsigned long out = 0;
    if (dst == 0 || dst_size == 0) return;
    dst[0] = '\0';
    if (topic == 0) return;
    for (i = 0; topic[i] != '\0' && out + 1 < dst_size; ++i)
    {
        unsigned char ch = (unsigned char)topic[i];
        if (isalnum(ch)) dst[out++] = (char)tolower(ch);
        else if (out > 0 && dst[out - 1] != '-') dst[out++] = '-';
    }
    while (out > 0 && dst[out - 1] == '-') out--;
    dst[out] = '\0';
    if (dst[0] == '\0') (void)snprintf(dst, dst_size, "topic");
}

static void nx_lc_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = NX_SEP;
    ls = strlen(sep);
    if (la > SIZE_MAX - ls || la + ls > SIZE_MAX - lb) return;
    total = la + ls + lb;
    if (total + 1U > (size_t)dst_size || total + 1U > sizeof(tmp)) return;
    if (la > 0U) memcpy(tmp, a, la);
    if (ls > 0U) memcpy(tmp + la, sep, ls);
    if (lb > 0U) memcpy(tmp + la + ls, b, lb);
    tmp[total] = '\0';
    memmove(dst, tmp, total + 1U);
    return;
}

static void nx_lc_mkdir_best_effort(const char* path)
{
    if (path != 0 && path[0] != '\0') (void)NX_MKDIR(path);
}

static int nx_lc_file_exists(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (f == 0) return 0;
    fclose(f);
    return 1;
}

static int nx_lc_read_file_prefix(const char* path, char* buffer, unsigned long buffer_size)
{
    FILE* f;
    size_t n;
    if (path == 0 || buffer == 0 || buffer_size == 0) return 0;
    f = fopen(path, "rb");
    if (f == 0) return 0;
    n = fread(buffer, 1, buffer_size - 1, f);
    buffer[n] = '\0';
    fclose(f);
    return 1;
}

static int nx_lc_write_fallback(const char* topic, const char* slug, NxLearningCoreResult* result)
{
    char knowledge_dir[512];
    char topics_dir[512];
    char topic_dir[512];
    FILE* memory;
    FILE* report;
    nx_lc_join(knowledge_dir, sizeof(knowledge_dir), ".", "Knowledge");
    nx_lc_join(topics_dir, sizeof(topics_dir), knowledge_dir, "Topics");
    nx_lc_join(topic_dir, sizeof(topic_dir), topics_dir, slug);
    nx_lc_mkdir_best_effort(knowledge_dir);
    nx_lc_mkdir_best_effort(topics_dir);
    nx_lc_mkdir_best_effort(topic_dir);
    nx_lc_join(result->memory_path, sizeof(result->memory_path), topic_dir, "memory.jsonl");
    nx_lc_join(result->report_path, sizeof(result->report_path), topic_dir, "report.md");
    (void)snprintf(result->output_dir, sizeof(result->output_dir), "%s", topic_dir);
    memory = fopen(result->memory_path, "wb");
    if (memory == 0) return 0;
    fprintf(memory, "{\"type\":\"topic\",\"name\":\"%s\",\"confidence\":35,\"source\":\"fallback\"}\n", topic);
    fprintf(memory, "{\"type\":\"fact\",\"topic\":\"%s\",\"text\":\"No se pudo adquirir informacion externa. Se creo memoria minima para el tema.\",\"confidence\":35}\n", topic);
    fclose(memory);
    report = fopen(result->report_path, "wb");
    if (report == 0) return 0;
    fprintf(report, "# Investigacion: %s\n\n", topic);
    fprintf(report, "Estado: memoria minima creada. No se pudo adquirir informacion externa desde PowerShell.\n");
    fclose(report);
    result->facts_written = 2;
    result->sources_seen = 0;
    result->confidence = 35;
    return 1;
}

NxLearningCoreStatus NxLearningCore_Learn(const char* topic, NxLearningCoreResult* result_out)
{
    char clean_topic[128];
    char slug[128];
    char command[2048];
    int rc;
    if (topic == 0 || result_out == 0) return NX_LEARNING_CORE_INVALID_ARGUMENT;
    memset(result_out, 0, sizeof(*result_out));
    nx_lc_copy_trimmed(clean_topic, sizeof(clean_topic), topic);
    if (clean_topic[0] == '\0') return NX_LEARNING_CORE_INVALID_ARGUMENT;
    nx_lc_slugify(slug, sizeof(slug), clean_topic);
    (void)snprintf(result_out->topic, sizeof(result_out->topic), "%s", clean_topic);
    (void)snprintf(result_out->topic_slug, sizeof(result_out->topic_slug), "%s", slug);
    printf("================================================\n");
    printf(" NEXIORA - Nucleo de aprendizaje real\n");
    printf("================================================\n\n");
    printf("Tema: %s\n\n", clean_topic);
    printf("[##..................]  10%% Preparando adquisicion\n");
    printf("[#####...............]  25%% Ejecutando conectores\n");
    fflush(stdout);
#if defined(_WIN32)
    (void)snprintf(command, sizeof(command), "powershell -NoProfile -ExecutionPolicy Bypass -File .\\Scripts\\nexiora-learn.ps1 -Topic \"%s\"", clean_topic);
#else
    (void)snprintf(command, sizeof(command), "pwsh -NoProfile -ExecutionPolicy Bypass -File ./Scripts/nexiora-learn.ps1 -Topic \"%s\"", clean_topic);
#endif
    rc = system(command);
    if (rc != 0)
    {
        printf("[########............]  40%% Conector externo no disponible; creando memoria minima\n");
        if (!nx_lc_write_fallback(clean_topic, slug, result_out)) return NX_LEARNING_CORE_SCRIPT_FAILED;
        printf("[####################] 100%% Memoria minima creada\n");
        return NX_LEARNING_CORE_SCRIPT_FAILED;
    }
    {
        char knowledge_dir[512];
        char topics_dir[512];
        char topic_dir[512];
        nx_lc_join(knowledge_dir, sizeof(knowledge_dir), ".", "Knowledge");
        nx_lc_join(topics_dir, sizeof(topics_dir), knowledge_dir, "Topics");
        nx_lc_join(topic_dir, sizeof(topic_dir), topics_dir, slug);
        nx_lc_join(result_out->memory_path, sizeof(result_out->memory_path), topic_dir, "memory.jsonl");
        nx_lc_join(result_out->report_path, sizeof(result_out->report_path), topic_dir, "report.md");
        (void)snprintf(result_out->output_dir, sizeof(result_out->output_dir), "%s", topic_dir);
    }
    if (!nx_lc_file_exists(result_out->memory_path) || !nx_lc_file_exists(result_out->report_path)) return NX_LEARNING_CORE_IO_FAILED;
    result_out->facts_written = 1;
    result_out->sources_seen = 1;
    result_out->confidence = 70;
    printf("[####################] 100%% Aprendizaje registrado\n\n");
    printf("Artefactos:\n  %s\n  %s\n", result_out->report_path, result_out->memory_path);
    return NX_LEARNING_CORE_OK;
}

NxLearningCoreStatus NxLearningCore_Query(const char* topic, char* buffer, unsigned long buffer_size)
{
    char clean_topic[128];
    char slug[128];
    char knowledge_dir[512];
    char topics_dir[512];
    char topic_dir[512];
    char answer_path[512];
    char report_path[512];
    if (topic == 0 || buffer == 0 || buffer_size == 0) return NX_LEARNING_CORE_INVALID_ARGUMENT;
    buffer[0] = '\0';
    nx_lc_copy_trimmed(clean_topic, sizeof(clean_topic), topic);
    if (clean_topic[0] == '\0') return NX_LEARNING_CORE_INVALID_ARGUMENT;
    nx_lc_slugify(slug, sizeof(slug), clean_topic);
    nx_lc_join(knowledge_dir, sizeof(knowledge_dir), ".", "Knowledge");
    nx_lc_join(topics_dir, sizeof(topics_dir), knowledge_dir, "Topics");
    nx_lc_join(topic_dir, sizeof(topic_dir), topics_dir, slug);
    nx_lc_join(answer_path, sizeof(answer_path), topic_dir, "answer.txt");
    nx_lc_join(report_path, sizeof(report_path), topic_dir, "report.md");
    if (nx_lc_read_file_prefix(answer_path, buffer, buffer_size)) return NX_LEARNING_CORE_OK;
    if (nx_lc_read_file_prefix(report_path, buffer, buffer_size)) return NX_LEARNING_CORE_OK;
    (void)snprintf(buffer, buffer_size, "No tengo conocimiento suficiente sobre '%s'. Ejecuta primero: nexiora aprende %s\n", clean_topic, clean_topic);
    return NX_LEARNING_CORE_NOT_FOUND;
}

const char* NxLearningCore_StatusToString(NxLearningCoreStatus status)
{
    switch (status)
    {
    case NX_LEARNING_CORE_OK: return "ok";
    case NX_LEARNING_CORE_INVALID_ARGUMENT: return "invalid argument";
    case NX_LEARNING_CORE_SCRIPT_FAILED: return "external acquisition failed";
    case NX_LEARNING_CORE_NOT_FOUND: return "not found";
    case NX_LEARNING_CORE_IO_FAILED: return "io failed";
    default: return "unknown";
    }
}
