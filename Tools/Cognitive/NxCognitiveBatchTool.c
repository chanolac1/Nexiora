#include "Nexiora/Cognitive/NxCognitiveCore.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

typedef struct BatchStats
{
    unsigned long scanned;
    unsigned long ingested;
    unsigned long skipped;
    unsigned long failed;
    unsigned long concepts;
    unsigned long chunks;
} BatchStats;

static int ends_with_ci(const char* s, const char* suffix)
{
    size_t a;
    size_t b;
    size_t i;
    if (s == NULL || suffix == NULL) return 0;
    a = strlen(s);
    b = strlen(suffix);
    if (b > a) return 0;
    for (i = 0; i < b; ++i)
    {
        char ca = (char)tolower((unsigned char)s[a - b + i]);
        char cb = (char)tolower((unsigned char)suffix[i]);
        if (ca != cb) return 0;
    }
    return 1;
}

static int is_supported_text_file(const char* path)
{
    return ends_with_ci(path, ".txt") ||
           ends_with_ci(path, ".md") ||
           ends_with_ci(path, ".markdown") ||
           ends_with_ci(path, ".srt") ||
           ends_with_ci(path, ".vtt") ||
           ends_with_ci(path, ".csv") ||
           ends_with_ci(path, ".json");
}

static void join_path(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep;
    size_t la;
    if (dst == NULL || dst_size == 0) return;
    dst[0] = '\0';
    if (a == NULL) a = "";
    if (b == NULL) b = "";
    la = strlen(a);
    sep = (la > 0 && (a[la - 1] == '/' || a[la - 1] == '\\')) ? "" : "\\";
#ifdef _WIN32
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b);
#else
    sep = (la > 0 && a[la - 1] == '/') ? "" : "/";
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b);
#endif
    dst[dst_size - 1] = '\0';
}

static void print_progress(const char* file, const BatchStats* stats)
{
    printf("\n------------------------------------------------\n");
    printf("Archivo: %s\n", file);
    printf("Escaneados: %lu | Ingeridos: %lu | Omitidos: %lu | Fallidos: %lu\n",
        stats->scanned, stats->ingested, stats->skipped, stats->failed);
}

static void ingest_one(const char* topic, const char* path, BatchStats* stats)
{
    NxCognitiveIngestResult result;
    NxCognitiveStatus status;

    stats->scanned++;
    if (!is_supported_text_file(path))
    {
        stats->skipped++;
        return;
    }

    print_progress(path, stats);
    status = NxCognitive_IngestFile(".", topic, path, &result);
    if (status == NX_COGNITIVE_OK)
    {
        stats->ingested++;
        stats->concepts += (unsigned long)result.concepts_written;
        stats->chunks += (unsigned long)result.chunks_written;
        printf("OK: conceptos=%lu fragmentos=%lu\n",
            (unsigned long)result.concepts_written,
            (unsigned long)result.chunks_written);
    }
    else
    {
        stats->failed++;
        printf("ERROR: %s\n", NxCognitive_StatusToString(status));
    }
}

#ifdef _WIN32
static void scan_dir(const char* topic, const char* dir, int recursive, BatchStats* stats)
{
    char pattern[1024];
    WIN32_FIND_DATAA data;
    HANDLE h;
    join_path(pattern, sizeof(pattern), dir, "*");
    h = FindFirstFileA(pattern, &data);
    if (h == INVALID_HANDLE_VALUE)
    {
        stats->failed++;
        printf("No pude abrir carpeta: %s\n", dir);
        return;
    }
    do
    {
        char path[1024];
        if (strcmp(data.cFileName, ".") == 0 || strcmp(data.cFileName, "..") == 0)
        {
            continue;
        }
        join_path(path, sizeof(path), dir, data.cFileName);
        if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            if (recursive) scan_dir(topic, path, recursive, stats);
        }
        else
        {
            ingest_one(topic, path, stats);
        }
    } while (FindNextFileA(h, &data));
    FindClose(h);
}
#else
static void scan_dir(const char* topic, const char* dir, int recursive, BatchStats* stats)
{
    DIR* d = opendir(dir);
    struct dirent* ent;
    if (!d)
    {
        stats->failed++;
        printf("No pude abrir carpeta: %s\n", dir);
        return;
    }
    while ((ent = readdir(d)) != NULL)
    {
        char path[1024];
        struct stat st;
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        join_path(path, sizeof(path), dir, ent->d_name);
        if (stat(path, &st) != 0) continue;
        if (S_ISDIR(st.st_mode))
        {
            if (recursive) scan_dir(topic, path, recursive, stats);
        }
        else
        {
            ingest_one(topic, path, stats);
        }
    }
    closedir(d);
}
#endif

static void usage(void)
{
    printf("Nexiora Cognitive Batch Ingest\n\n");
    printf("Uso:\n");
    printf("  nexiora_cognitive_batch ingest-dir <tema> <carpeta> [--recursive]\n");
    printf("  nexiora_cognitive_batch ask <tema> \"<pregunta>\"\n\n");
    printf("Ejemplo:\n");
    printf("  nexiora_cognitive_batch ingest-dir Genexus Samples\\Cognitive\\Batch --recursive\n");
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        usage();
        return 0;
    }

    if (strcmp(argv[1], "ingest-dir") == 0)
    {
        BatchStats stats;
        int recursive;
        if (argc < 4)
        {
            usage();
            return 1;
        }
        memset(&stats, 0, sizeof(stats));
        recursive = (argc >= 5 && strcmp(argv[4], "--recursive") == 0);
        printf("================================================\n");
        printf(" NEXIORA - Ingesta cognitiva de carpeta\n");
        printf("================================================\n\n");
        printf("Tema    : %s\n", argv[2]);
        printf("Carpeta : %s\n", argv[3]);
        printf("Recursivo: %s\n", recursive ? "si" : "no");
        scan_dir(argv[2], argv[3], recursive, &stats);
        printf("\n================================================\n");
        printf("Resumen\n");
        printf("================================================\n");
        printf("Archivos escaneados : %lu\n", stats.scanned);
        printf("Archivos ingeridos  : %lu\n", stats.ingested);
        printf("Archivos omitidos   : %lu\n", stats.skipped);
        printf("Errores             : %lu\n", stats.failed);
        printf("Conceptos acumulados: %lu\n", stats.concepts);
        printf("Fragmentos escritos : %lu\n", stats.chunks);
        printf("\nAhora puedes consultar:\n");
        printf("  nexiora_cognitive_batch ask %s \"Que aprendiste?\"\n", argv[2]);
        return stats.failed == 0 ? 0 : 2;
    }

    if (strcmp(argv[1], "ask") == 0)
    {
        NxCognitiveAnswer answer;
        NxCognitiveStatus status;
        if (argc < 4)
        {
            usage();
            return 1;
        }
        status = NxCognitive_Ask(".", argv[2], argv[3], &answer);
        if (status != NX_COGNITIVE_OK)
        {
            fprintf(stderr, "Pregunta fallida: %s\n", NxCognitive_StatusToString(status));
            return 3;
        }
        printf("%s\n", answer.answer);
        return 0;
    }

    usage();
    return 1;
}
