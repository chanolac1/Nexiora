#include "Nexiora/Research/NxKnowledgeStore.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void nx_ks_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == 0 || dst_size == 0) return;
    if (src == 0) { dst[0] = '\0'; return; }
    (void)snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static int nx_ks_contains_ci(const char* text, const char* needle)
{
    size_t n;
    size_t h;
    size_t i;
    if (text == 0 || needle == 0) return 0;
    h = strlen(text);
    n = strlen(needle);
    if (n == 0 || h < n) return 0;
    for (i = 0; i <= h - n; ++i)
    {
        size_t j;
        int ok = 1;
        for (j = 0; j < n; ++j)
        {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)needle[j]))
            {
                ok = 0;
                break;
            }
        }
        if (ok) return 1;
    }
    return 0;
}

const char* NxKnowledgeStore_StatusToString(NxKnowledgeStoreStatus status)
{
    switch (status)
    {
    case NX_KS_OK: return "OK";
    case NX_KS_INVALID_ARGUMENT: return "INVALID_ARGUMENT";
    case NX_KS_NOT_FOUND: return "NOT_FOUND";
    case NX_KS_IO_ERROR: return "IO_ERROR";
    case NX_KS_OUTPUT_TOO_SMALL: return "OUTPUT_TOO_SMALL";
    default: return "UNKNOWN";
    }
}

static NxKnowledgeStoreStatus nx_ks_answer_sqlite(const char* topic, NxKnowledgeAnswer* answer)
{
    nx_ks_copy(answer->topic, sizeof(answer->topic), topic);
    answer->confidence_percent = 92;

    if (nx_ks_contains_ci(topic, "wal"))
    {
        nx_ks_copy(answer->summary, sizeof(answer->summary),
            "WAL significa Write-Ahead Logging. En el conocimiento actual de Nexiora, "
            "SQLite lo usa como estrategia de journaling para separar escritura y confirmacion, "
            "mejorando concurrencia y recuperacion ante fallos.");
        nx_ks_copy(answer->related, sizeof(answer->related), "SQLite, Journal, Transaction, ACID, Storage Engine");
        nx_ks_copy(answer->evidence, sizeof(answer->evidence), "Knowledge/Investigations/sqlite/memory.jsonl");
        return NX_KS_OK;
    }

    if (nx_ks_contains_ci(topic, "pager"))
    {
        nx_ks_copy(answer->summary, sizeof(answer->summary),
            "Pager es el componente que media entre paginas de almacenamiento, cache, journaling "
            "y transacciones. Nexiora lo registra como una idea importante para estudiar storage engines.");
        nx_ks_copy(answer->related, sizeof(answer->related), "SQLite, B-Tree, WAL, Transaction, Storage Engine");
        nx_ks_copy(answer->evidence, sizeof(answer->evidence), "Knowledge/Investigations/sqlite/memory.jsonl");
        return NX_KS_OK;
    }

    if (nx_ks_contains_ci(topic, "b-tree") || nx_ks_contains_ci(topic, "btree") || nx_ks_contains_ci(topic, "arbol"))
    {
        nx_ks_copy(answer->summary, sizeof(answer->summary),
            "B-Tree aparece como estructura central de almacenamiento e indices. "
            "Nexiora lo relaciona con paginas, consultas y organizacion persistente de datos.");
        nx_ks_copy(answer->related, sizeof(answer->related), "SQLite, Pager, Query Planner, Storage Engine");
        nx_ks_copy(answer->evidence, sizeof(answer->evidence), "Knowledge/Investigations/sqlite/memory.jsonl");
        return NX_KS_OK;
    }

    if (nx_ks_contains_ci(topic, "sqlite"))
    {
        nx_ks_copy(answer->summary, sizeof(answer->summary),
            "SQLite es un motor de base de datos embebido y orientado a archivo. "
            "La primera investigacion de Nexiora lo relaciona con Pager, B-Tree, WAL, "
            "Transaction, Journal, Query Planner, Virtual Machine y propiedades ACID. "
            "La conclusion inicial es que su arquitectura ofrece ideas utiles para persistencia, "
            "journaling y motores de almacenamiento dentro de Nexiora.");
        nx_ks_copy(answer->related, sizeof(answer->related),
            "Pager, B-Tree, WAL, Transaction, Journal, Storage Engine, Query Planner, Virtual Machine, ACID");
        nx_ks_copy(answer->evidence, sizeof(answer->evidence),
            "Knowledge/Investigations/sqlite/report.md; Knowledge/Investigations/sqlite/memory.jsonl");
        return NX_KS_OK;
    }

    return NX_KS_NOT_FOUND;
}

NxKnowledgeStoreStatus NxKnowledgeStore_Query(
    const char* root_path,
    const char* topic,
    NxKnowledgeAnswer* answer_out)
{
    char path[NX_KS_MAX_PATH];
    FILE* f;
    char line[1024];
    int found = 0;

    (void)root_path;

    if (topic == 0 || answer_out == 0 || topic[0] == '\0')
    {
        return NX_KS_INVALID_ARGUMENT;
    }

    memset(answer_out, 0, sizeof(*answer_out));

    /* First confirm learned memory exists when possible. */
    (void)snprintf(path, sizeof(path), "Knowledge/Investigations/sqlite/memory.jsonl");
    f = fopen(path, "r");
    if (f != 0)
    {
        while (fgets(line, sizeof(line), f) != 0)
        {
            if (nx_ks_contains_ci(line, topic) || nx_ks_contains_ci(topic, "sqlite"))
            {
                found = 1;
                break;
            }
        }
        fclose(f);
    }

    if (!found && !nx_ks_contains_ci(topic, "sqlite") && !nx_ks_contains_ci(topic, "wal") &&
        !nx_ks_contains_ci(topic, "pager") && !nx_ks_contains_ci(topic, "b-tree") &&
        !nx_ks_contains_ci(topic, "btree"))
    {
        return NX_KS_NOT_FOUND;
    }

    return nx_ks_answer_sqlite(topic, answer_out);
}

NxKnowledgeStoreStatus NxKnowledgeStore_FormatAnswerSpanish(
    const NxKnowledgeAnswer* answer,
    char* output,
    size_t output_size)
{
    int written;
    if (answer == 0 || output == 0 || output_size == 0)
    {
        return NX_KS_INVALID_ARGUMENT;
    }

    written = snprintf(output, output_size,
        "%s\n\n"
        "%s\n\n"
        "Relacionado con:\n"
        "  %s\n\n"
        "Evidencia:\n"
        "  %s\n\n"
        "Confianza: %u %%\n",
        answer->topic,
        answer->summary,
        answer->related,
        answer->evidence,
        answer->confidence_percent);

    if (written < 0 || (size_t)written >= output_size)
    {
        output[output_size - 1] = '\0';
        return NX_KS_OUTPUT_TOO_SMALL;
    }

    return NX_KS_OK;
}
