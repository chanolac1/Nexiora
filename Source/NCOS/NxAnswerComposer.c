#include "Nexiora/NCOS/NxAnswerComposer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0755)
#endif

static void nx_clear_result(NxAnswerComposerResult* r)
{
    if (r) memset(r, 0, sizeof(*r));
}

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static void nx_lower_token(char* dst, size_t dst_size, const char* src)
{
    size_t i = 0;
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    while (*src && i + 1 < dst_size)
    {
        unsigned char c = (unsigned char)*src++;
        if (isalnum(c)) dst[i++] = (char)tolower(c);
        else if ((c == '_' || c == '-' || c == ' ') && i > 0 && dst[i - 1] != '_') dst[i++] = '_';
    }
    while (i > 0 && dst[i - 1] == '_') --i;
    dst[i] = '\0';
}

static void nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t len;
    if (!dst || dst_size == 0) return;
    if (!a) a = "";
    if (!b) b = "";
    len = strlen(a);
    if (len > 0 && a[len - 1] != '/' && a[len - 1] != '\\') sep = "/";
    snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    dst[dst_size - 1] = '\0';
}

static int nx_read_file(const char* path, char* buffer, size_t buffer_size)
{
    FILE* f;
    size_t n;
    if (!path || !buffer || buffer_size == 0) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    n = fread(buffer, 1, buffer_size - 1, f);
    buffer[n] = '\0';
    fclose(f);
    return 1;
}

static int nx_extract_field(const char* text, const char* key, char* out, size_t out_size)
{
    char pattern[128];
    const char* p;
    const char* start;
    const char* end;
    if (!text || !key || !out || out_size == 0) return 0;
    snprintf(pattern, sizeof(pattern), "%s:", key);
    p = strstr(text, pattern);
    if (!p) return 0;
    start = p + strlen(pattern);
    while (*start == ' ' || *start == '\t') start++;
    end = start;
    while (*end && *end != '\n' && *end != '\r') end++;
    if ((size_t)(end - start) >= out_size) end = start + out_size - 1;
    memcpy(out, start, (size_t)(end - start));
    out[end - start] = '\0';
    return 1;
}

static void nx_concept_card_path(char* dst, size_t dst_size, const char* root, const char* domain, const char* concept)
{
    char knowledge[512];
    char ncos[512];
    char concepts[512];
    char domain_token[128];
    char concept_token[128];
    char file[160];
    nx_lower_token(domain_token, sizeof(domain_token), domain);
    nx_lower_token(concept_token, sizeof(concept_token), concept);
    snprintf(file, sizeof(file), "%s.card", concept_token);
    nx_join(knowledge, sizeof(knowledge), root ? root : ".", "Knowledge");
    nx_join(ncos, sizeof(ncos), knowledge, "NCOS");
    nx_join(concepts, sizeof(concepts), ncos, "Concepts");
    nx_join(concepts, sizeof(concepts), concepts, domain_token);
    nx_join(dst, dst_size, concepts, file);
}

static void nx_graph_path(char* dst, size_t dst_size, const char* root, const char* domain)
{
    char knowledge[512];
    char ncos[512];
    char graphs[512];
    char domain_token[128];
    nx_lower_token(domain_token, sizeof(domain_token), domain);
    nx_join(knowledge, sizeof(knowledge), root ? root : ".", "Knowledge");
    nx_join(ncos, sizeof(ncos), knowledge, "NCOS");
    nx_join(graphs, sizeof(graphs), ncos, "ConceptGraphs");
    nx_join(graphs, sizeof(graphs), graphs, domain_token);
    nx_join(dst, dst_size, graphs, "edges.jsonl");
}

static int nx_find_direct_edge(const char* edges, const char* from, const char* to, char* relation, size_t relation_size)
{
    char from_token[128], to_token[128];
    const char* line = edges;
    nx_lower_token(from_token, sizeof(from_token), from);
    nx_lower_token(to_token, sizeof(to_token), to);
    while (line && *line)
    {
        const char* next = strchr(line, '\n');
        char row[512];
        char* f;
        char* r;
        char* t;
        size_t len = next ? (size_t)(next - line) : strlen(line);
        if (len >= sizeof(row)) len = sizeof(row) - 1;
        memcpy(row, line, len);
        row[len] = '\0';
        f = strtok(row, "|");
        r = strtok(NULL, "|");
        t = strtok(NULL, "|");
        if (f && r && t && strcmp(f, from_token) == 0 && strcmp(t, to_token) == 0)
        {
            nx_copy(relation, relation_size, r);
            return 1;
        }
        line = next ? next + 1 : NULL;
    }
    return 0;
}

static int nx_find_indirect_edge(const char* edges, const char* from, const char* to, char* mid, size_t mid_size, char* r1, size_t r1_size, char* r2, size_t r2_size)
{
    char from_token[128], to_token[128];
    const char* line = edges;
    nx_lower_token(from_token, sizeof(from_token), from);
    nx_lower_token(to_token, sizeof(to_token), to);
    while (line && *line)
    {
        const char* next = strchr(line, '\n');
        char row[512], f1[128], m1[128], rel1[128];
        size_t len = next ? (size_t)(next - line) : strlen(line);
        char* f; char* r; char* t;
        if (len >= sizeof(row)) len = sizeof(row) - 1;
        memcpy(row, line, len); row[len] = '\0';
        f = strtok(row, "|"); r = strtok(NULL, "|"); t = strtok(NULL, "|");
        if (f && r && t && strcmp(f, from_token) == 0)
        {
            nx_copy(f1, sizeof(f1), f); nx_copy(rel1, sizeof(rel1), r); nx_copy(m1, sizeof(m1), t);
            const char* line2 = edges;
            while (line2 && *line2)
            {
                const char* next2 = strchr(line2, '\n');
                char row2[512];
                size_t len2 = next2 ? (size_t)(next2 - line2) : strlen(line2);
                char* f2; char* r_2; char* t2;
                if (len2 >= sizeof(row2)) len2 = sizeof(row2) - 1;
                memcpy(row2, line2, len2); row2[len2] = '\0';
                f2 = strtok(row2, "|"); r_2 = strtok(NULL, "|"); t2 = strtok(NULL, "|");
                if (f2 && r_2 && t2 && strcmp(f2, m1) == 0 && strcmp(t2, to_token) == 0)
                {
                    nx_copy(mid, mid_size, m1); nx_copy(r1, r1_size, rel1); nx_copy(r2, r2_size, r_2); return 1;
                }
                line2 = next2 ? next2 + 1 : NULL;
            }
        }
        line = next ? next + 1 : NULL;
    }
    return 0;
}

int NxAnswerComposer_ComposeRelationAnswer(const char* root_path,
                                           const char* domain,
                                           const char* source_concept,
                                           const char* target_concept,
                                           NxAnswerComposerResult* result_out)
{
    char source_path[512], target_path[512], graph_path[512];
    char source_card[8192], target_card[8192], edges[16384];
    char source_def[1024] = "", source_purpose[1024] = "";
    char target_def[1024] = "", target_purpose[1024] = "";
    char relation[128] = "", mid[128] = "", r1[128] = "", r2[128] = "";
    int direct;
    int indirect;

    if (!result_out || !domain || !source_concept || !target_concept) return 0;
    nx_clear_result(result_out);
    nx_copy(result_out->domain, sizeof(result_out->domain), domain);
    nx_copy(result_out->source, sizeof(result_out->source), source_concept);
    nx_copy(result_out->target, sizeof(result_out->target), target_concept);
    nx_copy(result_out->intent, sizeof(result_out->intent), "relacion");

    nx_concept_card_path(source_path, sizeof(source_path), root_path, domain, source_concept);
    nx_concept_card_path(target_path, sizeof(target_path), root_path, domain, target_concept);
    nx_graph_path(graph_path, sizeof(graph_path), root_path, domain);

    if (!nx_read_file(source_path, source_card, sizeof(source_card)))
    {
        snprintf(result_out->answer, sizeof(result_out->answer), "No encontre tarjeta de concepto para '%s'.", source_concept);
        result_out->confidence = 0;
        return 1;
    }
    if (!nx_read_file(target_path, target_card, sizeof(target_card))) target_card[0] = '\0';
    if (!nx_read_file(graph_path, edges, sizeof(edges))) edges[0] = '\0';

    nx_extract_field(source_card, "definition", source_def, sizeof(source_def));
    nx_extract_field(source_card, "purpose", source_purpose, sizeof(source_purpose));
    nx_extract_field(target_card, "definition", target_def, sizeof(target_def));
    nx_extract_field(target_card, "purpose", target_purpose, sizeof(target_purpose));

    direct = nx_find_direct_edge(edges, source_concept, target_concept, relation, sizeof(relation));
    indirect = direct ? 0 : nx_find_indirect_edge(edges, source_concept, target_concept, mid, sizeof(mid), r1, sizeof(r1), r2, sizeof(r2));

    if (direct)
    {
        snprintf(result_out->answer, sizeof(result_out->answer),
                 "Respuesta estructurada:\n\nDefinicion de %s:\n%s\n\nProposito:\n%s\n\nRelacion detectada:\n%s se relaciona con %s mediante '%s'.\n\nLectura de Nexiora:\nEsta relacion indica que ambos conceptos pertenecen al mismo modelo de conocimiento y deben considerarse juntos al responder preguntas de diseno o implementacion.",
                 source_concept, source_def[0] ? source_def : "Sin definicion registrada.",
                 source_purpose[0] ? source_purpose : "Sin proposito registrado.",
                 source_concept, target_concept, relation);
        snprintf(result_out->evidence, sizeof(result_out->evidence),
                 "- Tarjeta: %s\n- Tarjeta: %s\n- Grafo: %s --%s--> %s\n",
                 source_path, target_path, source_concept, relation, target_concept);
        result_out->confidence = 88;
        result_out->evidence_count = 3;
        return 1;
    }

    if (indirect)
    {
        snprintf(result_out->answer, sizeof(result_out->answer),
                 "Respuesta estructurada:\n\nDefinicion de %s:\n%s\n\nProposito:\n%s\n\nRelacion inferida:\n%s se conecta con %s a traves de %s.\n\nCadena de razonamiento:\n%s --%s--> %s --%s--> %s\n\nLectura de Nexiora:\nLa relacion es indirecta, por lo tanto la confianza es menor que una relacion directa, pero es util para explicar dependencias conceptuales.",
                 source_concept, source_def[0] ? source_def : "Sin definicion registrada.",
                 source_purpose[0] ? source_purpose : "Sin proposito registrado.",
                 source_concept, target_concept, mid,
                 source_concept, r1, mid, r2, target_concept);
        snprintf(result_out->evidence, sizeof(result_out->evidence),
                 "- Tarjeta: %s\n- Grafo: %s --%s--> %s\n- Grafo: %s --%s--> %s\n",
                 source_path, source_concept, r1, mid, mid, r2, target_concept);
        result_out->confidence = 78;
        result_out->evidence_count = 3;
        return 1;
    }

    snprintf(result_out->answer, sizeof(result_out->answer),
             "Respuesta estructurada:\n\nDefinicion de %s:\n%s\n\nNo encontre una relacion directa o indirecta registrada entre %s y %s.\n\nRecomendacion:\nCrear o investigar relaciones antes de emitir una conclusion fuerte.",
             source_concept, source_def[0] ? source_def : "Sin definicion registrada.", source_concept, target_concept);
    snprintf(result_out->evidence, sizeof(result_out->evidence), "- Tarjeta: %s\n- Grafo consultado: %s\n", source_path, graph_path);
    result_out->confidence = 35;
    result_out->evidence_count = 1;
    return 1;
}

int NxAnswerComposer_WriteAnswer(const char* path, const NxAnswerComposerResult* result)
{
    FILE* f;
    if (!path || !result) return 0;
    f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "Dominio: %s\n", result->domain);
    fprintf(f, "Origen: %s\n", result->source);
    fprintf(f, "Destino: %s\n", result->target);
    fprintf(f, "Intencion: %s\n\n", result->intent);
    fprintf(f, "%s\n\n", result->answer);
    fprintf(f, "Evidencia usada:\n%s\n", result->evidence);
    fprintf(f, "Confianza: %d %%\n", result->confidence);
    fclose(f);
    return 1;
}
