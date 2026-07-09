#include "Nexiora/NCOS/NxConceptGraph.h"

#include <ctype.h>
#include <stdio.h>
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

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

void NxConceptGraph_Normalize(const char* input, char* output, size_t output_size)
{
    size_t j = 0;
    int last_sep = 0;

    if (output == NULL || output_size == 0) return;
    output[0] = '\0';
    if (input == NULL) return;

    for (size_t i = 0; input[i] != '\0' && j + 1 < output_size; ++i)
    {
        unsigned char ch = (unsigned char)input[i];
        if (isalnum(ch))
        {
            output[j++] = (char)tolower(ch);
            last_sep = 0;
        }
        else if (!last_sep && j > 0)
        {
            output[j++] = '_';
            last_sep = 1;
        }
    }

    while (j > 0 && output[j - 1] == '_') --j;
    output[j] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    if (dst == NULL || dst_size == 0 || a == NULL || b == NULL) return 0;
    if (a[0] != '\0')
    {
        size_t n = strlen(a);
        if (n > 0 && a[n - 1] != '/' && a[n - 1] != '\\') sep = "/";
    }
    int written = snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    if (written < 0 || (size_t)written >= dst_size)
    {
        dst[dst_size - 1] = '\0';
        return 0;
    }
    return 1;
}

static int nx_mkdir_one(const char* path)
{
    if (path == NULL || path[0] == '\0') return 0;
    if (NX_MKDIR(path) == 0) return 1;
    return 1; /* Existing directories are acceptable for this engine. */
}

static int nx_mkdirs(const char* path)
{
    char tmp[512];
    size_t len;

    if (path == NULL || path[0] == '\0') return 0;
    nx_copy(tmp, sizeof(tmp), path);
    len = strlen(tmp);
    if (len == 0) return 0;

    for (size_t i = 1; i < len; ++i)
    {
        if (tmp[i] == '/' || tmp[i] == '\\')
        {
            char saved = tmp[i];
            tmp[i] = '\0';
            if (tmp[0] != '\0') nx_mkdir_one(tmp);
            tmp[i] = saved;
        }
    }
    nx_mkdir_one(tmp);
    return 1;
}

static int nx_graph_dir(const char* root, const char* domain, char* dir, size_t dir_size)
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
    if (!nx_join(dir, dir_size, graphs, norm_domain)) return 0;
    return nx_mkdirs(dir);
}

static int nx_edges_path(const char* root, const char* domain, char* path, size_t path_size)
{
    char dir[512];
    if (!nx_graph_dir(root, domain, dir, sizeof(dir))) return 0;
    return nx_join(path, path_size, dir, "edges.jsonl");
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

int NxConceptGraph_Find(
    const char* root,
    const char* domain,
    const char* from,
    const char* relation,
    const char* to,
    NxConceptGraphEdge* edge_out)
{
    char path[512];
    char line[1024];
    char nf[128], nr[128], nt[128];
    FILE* f;

    if (!nx_edges_path(root, domain, path, sizeof(path))) return 0;
    NxConceptGraph_Normalize(from, nf, sizeof(nf));
    NxConceptGraph_Normalize(relation, nr, sizeof(nr));
    NxConceptGraph_Normalize(to, nt, sizeof(nt));

    f = fopen(path, "r");
    if (f == NULL) return 0;

    while (fgets(line, sizeof(line), f) != NULL)
    {
        char lf[128] = {0};
        char lr[128] = {0};
        char lt[128] = {0};
        nx_parse_field(line, "from", lf, sizeof(lf));
        nx_parse_field(line, "relation", lr, sizeof(lr));
        nx_parse_field(line, "to", lt, sizeof(lt));
        if (strcmp(lf, nf) == 0 && strcmp(lr, nr) == 0 && strcmp(lt, nt) == 0)
        {
            if (edge_out != NULL)
            {
                memset(edge_out, 0, sizeof(*edge_out));
                NxConceptGraph_Normalize(domain, edge_out->domain, sizeof(edge_out->domain));
                nx_copy(edge_out->from, sizeof(edge_out->from), lf);
                nx_copy(edge_out->relation, sizeof(edge_out->relation), lr);
                nx_copy(edge_out->to, sizeof(edge_out->to), lt);
                nx_parse_field(line, "source", edge_out->source, sizeof(edge_out->source));
                nx_parse_int_field(line, "confidence", &edge_out->confidence);
                nx_copy(edge_out->path, sizeof(edge_out->path), path);
            }
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int NxConceptGraph_Link(
    const char* root,
    const char* domain,
    const char* from,
    const char* relation,
    const char* to,
    const char* source,
    int confidence,
    NxConceptGraphEdge* edge_out)
{
    char path[512];
    char nd[128], nf[128], nr[128], nt[128];
    FILE* f;

    if (confidence < 0) confidence = 0;
    if (confidence > 100) confidence = 100;

    if (NxConceptGraph_Find(root, domain, from, relation, to, edge_out)) return 1;
    if (!nx_edges_path(root, domain, path, sizeof(path))) return 0;

    NxConceptGraph_Normalize(domain, nd, sizeof(nd));
    NxConceptGraph_Normalize(from, nf, sizeof(nf));
    NxConceptGraph_Normalize(relation, nr, sizeof(nr));
    NxConceptGraph_Normalize(to, nt, sizeof(nt));
    if (nd[0] == '\0' || nf[0] == '\0' || nr[0] == '\0' || nt[0] == '\0') return 0;

    f = fopen(path, "a");
    if (f == NULL) return 0;
    fprintf(f,
            "{\"domain\":\"%s\",\"from\":\"%s\",\"relation\":\"%s\",\"to\":\"%s\",\"source\":\"%s\",\"confidence\":%d}\n",
            nd, nf, nr, nt, source ? source : "manual", confidence);
    fclose(f);

    if (edge_out != NULL)
    {
        memset(edge_out, 0, sizeof(*edge_out));
        nx_copy(edge_out->domain, sizeof(edge_out->domain), nd);
        nx_copy(edge_out->from, sizeof(edge_out->from), nf);
        nx_copy(edge_out->relation, sizeof(edge_out->relation), nr);
        nx_copy(edge_out->to, sizeof(edge_out->to), nt);
        nx_copy(edge_out->source, sizeof(edge_out->source), source ? source : "manual");
        edge_out->confidence = confidence;
        nx_copy(edge_out->path, sizeof(edge_out->path), path);
    }

    return 1;
}

int NxConceptGraph_Stats(
    const char* root,
    const char* domain,
    const char* concept,
    NxConceptGraphStats* stats_out)
{
    char path[512];
    char line[1024];
    char nc[128];
    FILE* f;

    if (stats_out == NULL) return 0;
    memset(stats_out, 0, sizeof(*stats_out));
    if (!nx_edges_path(root, domain, path, sizeof(path))) return 0;
    nx_copy(stats_out->path, sizeof(stats_out->path), path);
    NxConceptGraph_Normalize(concept, nc, sizeof(nc));

    f = fopen(path, "r");
    if (f == NULL) return 1;
    while (fgets(line, sizeof(line), f) != NULL)
    {
        char from_buf[128] = {0};
        char to_buf[128] = {0};
        nx_parse_field(line, "from", from_buf, sizeof(from_buf));
        nx_parse_field(line, "to", to_buf, sizeof(to_buf));
        stats_out->edge_count++;
        if (strcmp(from_buf, nc) == 0) stats_out->outgoing_count++;
        if (strcmp(to_buf, nc) == 0) stats_out->incoming_count++;
    }
    fclose(f);
    return 1;
}

int NxConceptGraph_Show(
    const char* root,
    const char* domain,
    const char* concept,
    char* output,
    size_t output_size)
{
    char path[512];
    char line[1024];
    char nc[128];
    size_t used = 0;
    FILE* f;

    if (output == NULL || output_size == 0) return 0;
    output[0] = '\0';
    if (!nx_edges_path(root, domain, path, sizeof(path))) return 0;
    NxConceptGraph_Normalize(concept, nc, sizeof(nc));

    used += (size_t)snprintf(output + used, output_size - used,
        "Grafo de conceptos\nDominio: %s\nConcepto: %s\n\nRelaciones salientes:\n",
        domain ? domain : "", nc);
    if (used >= output_size) { output[output_size - 1] = '\0'; return 1; }

    f = fopen(path, "r");
    if (f == NULL)
    {
        snprintf(output + used, output_size - used, "  (sin relaciones)\n");
        return 1;
    }

    int found = 0;
    while (fgets(line, sizeof(line), f) != NULL && used + 80 < output_size)
    {
        char from_buf[128] = {0};
        char rel_buf[128] = {0};
        char to_buf[128] = {0};
        int confidence = 0;
        nx_parse_field(line, "from", from_buf, sizeof(from_buf));
        nx_parse_field(line, "relation", rel_buf, sizeof(rel_buf));
        nx_parse_field(line, "to", to_buf, sizeof(to_buf));
        nx_parse_int_field(line, "confidence", &confidence);
        if (strcmp(from_buf, nc) == 0)
        {
            used += (size_t)snprintf(output + used, output_size - used,
                "  - %s -> %s  [confianza %d%%]\n", rel_buf, to_buf, confidence);
            found = 1;
        }
    }
    fclose(f);

    if (!found && used + 32 < output_size)
    {
        snprintf(output + used, output_size - used, "  (sin relaciones salientes)\n");
    }
    return 1;
}
