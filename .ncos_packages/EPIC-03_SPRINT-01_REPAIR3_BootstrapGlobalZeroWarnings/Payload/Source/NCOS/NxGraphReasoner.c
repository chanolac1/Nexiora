#include "Nexiora/NCOS/NxGraphReasoner.h"
#include "Nexiora/NCOS/NxConceptGraph.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

typedef struct NxReasonEdge
{
    char from[128];
    char relation[128];
    char to[128];
    int confidence;
} NxReasonEdge;

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t la, lb, ls, total;
    char tmp[1024];
    if (dst == NULL || dst_size == 0U || a == NULL || b == NULL) return 0;
    la = strlen(a); lb = strlen(b);
    if (la > 0U && a[la - 1U] != '/' && a[la - 1U] != '\\') sep = "/";
    ls = strlen(sep);
    if (la > SIZE_MAX - ls || la + ls > SIZE_MAX - lb) return 0;
    total = la + ls + lb;
    if (total + 1U > (size_t)dst_size || total + 1U > sizeof(tmp)) return 0;
    if (la > 0U) memcpy(tmp, a, la);
    if (ls > 0U) memcpy(tmp + la, sep, ls);
    if (lb > 0U) memcpy(tmp + la + ls, b, lb);
    tmp[total] = '\0';
    memmove(dst, tmp, total + 1U);
    return 1;
}

static int nx_edges_path(const char* root, const char* domain, char* path, size_t path_size)
{
    char knowledge[512];
    char ncos[512];
    char graphs[512];
    char norm_domain[128];

    NxConceptGraph_Normalize(domain, norm_domain, sizeof(norm_domain));
    if (norm_domain[0] == '\0') return 0;
    if (!nx_join(knowledge, sizeof(knowledge), root && root[0] ? root : ".", "Knowledge")) return 0;
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    if (!nx_join(graphs, sizeof(graphs), ncos, "ConceptGraphs")) return 0;
    if (!nx_join(graphs, sizeof(graphs), graphs, norm_domain)) return 0;
    return nx_join(path, path_size, graphs, "edges.jsonl");
}

static int nx_parse_field(const char* line, const char* key, char* out, size_t out_size)
{
    char pattern[64];
    const char* p;
    const char* q;
    snprintf(pattern, sizeof(pattern), "\"%s\":\"", key);
    p = strstr(line, pattern);
    if (p == NULL) return 0;
    p += strlen(pattern);
    q = strchr(p, '"');
    if (q == NULL) return 0;
    size_t n = (size_t)(q - p);
    if (n >= out_size) n = out_size - 1;
    memcpy(out, p, n);
    out[n] = '\0';
    return 1;
}

static int nx_parse_int_field(const char* line, const char* key, int* out)
{
    char pattern[64];
    const char* p;
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    p = strstr(line, pattern);
    if (p == NULL) return 0;
    p += strlen(pattern);
    *out = atoi(p);
    return 1;
}

static int nx_load_edges(const char* root, const char* domain, NxReasonEdge* edges, size_t max_edges, size_t* count_out)
{
    char path[512];
    char line[1024];
    FILE* f;
    size_t count = 0;

    if (count_out != NULL) *count_out = 0;
    if (edges == NULL || max_edges == 0) return 0;
    if (!nx_edges_path(root, domain, path, sizeof(path))) return 0;

    f = fopen(path, "r");
    if (f == NULL) return 0;

    while (fgets(line, sizeof(line), f) != NULL && count < max_edges)
    {
        NxReasonEdge e;
        memset(&e, 0, sizeof(e));
        if (nx_parse_field(line, "from", e.from, sizeof(e.from)) &&
            nx_parse_field(line, "relation", e.relation, sizeof(e.relation)) &&
            nx_parse_field(line, "to", e.to, sizeof(e.to)))
        {
            if (!nx_parse_int_field(line, "confidence", &e.confidence)) e.confidence = 60;
            edges[count++] = e;
        }
    }

    fclose(f);
    if (count_out != NULL) *count_out = count;
    return count > 0;
}

static void nx_append(char* out, size_t out_size, const char* text)
{
    size_t used;
    if (out == NULL || out_size == 0 || text == NULL) return;
    used = strlen(out);
    if (used + 1 >= out_size) return;
    snprintf(out + used, out_size - used, "%s", text);
}

static void nx_append_edge_line(char* out, size_t out_size, const NxReasonEdge* e)
{
    char line[384];
    snprintf(line, sizeof(line), "- %.120s --%.80s--> %.120s (confianza %d%%)\n", e->from, e->relation, e->to, e->confidence);
    nx_append(out, out_size, line);
}

int NxGraphReasoner_Explain(
    const char* root,
    const char* domain,
    const char* from,
    const char* to,
    NxGraphReasoningResult* result_out)
{
    NxReasonEdge edges[256];
    size_t edge_count = 0;
    char nf[128];
    char nt[128];
    int best_conf = 0;

    if (result_out == NULL) return 0;
    memset(result_out, 0, sizeof(*result_out));
    NxConceptGraph_Normalize(domain, result_out->domain, sizeof(result_out->domain));
    NxConceptGraph_Normalize(from, nf, sizeof(nf));
    NxConceptGraph_Normalize(to, nt, sizeof(nt));
    nx_copy(result_out->from, sizeof(result_out->from), nf);
    nx_copy(result_out->to, sizeof(result_out->to), nt);

    if (nf[0] == '\0' || nt[0] == '\0') return 0;
    if (!nx_load_edges(root, domain, edges, 256, &edge_count))
    {
        snprintf(result_out->answer, sizeof(result_out->answer),
                 "No encontre evidencia en el grafo para relacionar '%s' con '%s'.", nf, nt);
        result_out->confidence = 0;
        return 0;
    }

    for (size_t i = 0; i < edge_count; ++i)
    {
        if (strcmp(edges[i].from, nf) == 0 && strcmp(edges[i].to, nt) == 0)
        {
            result_out->found_direct = 1;
            result_out->evidence_count++;
            if (edges[i].confidence > best_conf) best_conf = edges[i].confidence;
            nx_append_edge_line(result_out->evidence, sizeof(result_out->evidence), &edges[i]);
        }
    }

    if (!result_out->found_direct)
    {
        for (size_t i = 0; i < edge_count; ++i)
        {
            if (strcmp(edges[i].from, nf) != 0) continue;
            for (size_t j = 0; j < edge_count; ++j)
            {
                if (strcmp(edges[i].to, edges[j].from) == 0 && strcmp(edges[j].to, nt) == 0)
                {
                    result_out->found_indirect = 1;
                    result_out->evidence_count += 2;
                    best_conf = (edges[i].confidence + edges[j].confidence) / 2;
                    nx_append_edge_line(result_out->evidence, sizeof(result_out->evidence), &edges[i]);
                    nx_append_edge_line(result_out->evidence, sizeof(result_out->evidence), &edges[j]);
                    i = edge_count;
                    break;
                }
            }
        }
    }

    if (result_out->found_direct)
    {
        snprintf(result_out->answer, sizeof(result_out->answer),
                 "Existe una relacion directa entre '%s' y '%s' en el grafo de conocimiento.", nf, nt);
        result_out->confidence = best_conf > 0 ? best_conf : 80;
        return 1;
    }

    if (result_out->found_indirect)
    {
        snprintf(result_out->answer, sizeof(result_out->answer),
                 "Existe una relacion indirecta entre '%s' y '%s' a traves de conceptos intermedios.", nf, nt);
        result_out->confidence = best_conf > 0 ? best_conf : 65;
        return 1;
    }

    snprintf(result_out->answer, sizeof(result_out->answer),
             "No encontre una ruta suficiente entre '%s' y '%s'.", nf, nt);
    result_out->confidence = 15;
    return 0;
}

int NxGraphReasoner_WhyRelated(
    const char* root,
    const char* domain,
    const char* from,
    const char* to,
    char* output,
    size_t output_size)
{
    NxGraphReasoningResult r;
    int ok;
    if (output == NULL || output_size == 0) return 0;
    output[0] = '\0';
    ok = NxGraphReasoner_Explain(root, domain, from, to, &r);

    snprintf(output, output_size,
             "Razonamiento sobre grafo de conceptos\n\nDominio: %s\nOrigen : %s\nDestino: %s\n\nRespuesta:\n%s\n\nEvidencia usada:\n%s\nConfianza: %d %%\n",
             r.domain, r.from, r.to, r.answer, r.evidence[0] ? r.evidence : "- Sin evidencia suficiente\n", r.confidence);
    return ok;
}
