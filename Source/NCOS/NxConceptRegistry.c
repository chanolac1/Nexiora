#include "Nexiora/NCOS/NxConceptRegistry.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static void nx_copy(char* dst, size_t dst_size, const char* src)
{
    if (dst == NULL || dst_size == 0) return;
    if (src == NULL) src = "";
    snprintf(dst, dst_size, "%s", src);
    dst[dst_size - 1] = '\0';
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    if (dst == NULL || dst_size == 0 || a == NULL || b == NULL) return 0;
    const char* sep = "";
    size_t n = strlen(a);
    if (n > 0 && a[n - 1] != '/' && a[n - 1] != '\\') sep = "/";
    int written = snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    if (written < 0 || (size_t)written >= dst_size) return 0;
    return 1;
}

static void nx_make_dir(const char* path)
{
    if (path != NULL && path[0] != '\0') (void)NX_MKDIR(path);
}

static int nx_ensure_root(const char* root)
{
    if (root == NULL || root[0] == '\0') return 0;
    nx_make_dir(root);
    return 1;
}

static int nx_make_concept_dirs(const char* root, const char* domain_normalized, char* domain_dir, size_t domain_dir_size)
{
    char knowledge[512];
    char ncos[512];
    char concepts[512];
    if (!nx_ensure_root(root)) return 0;
    if (!nx_join(knowledge, sizeof(knowledge), root, "Knowledge")) return 0;
    nx_make_dir(knowledge);
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    nx_make_dir(ncos);
    if (!nx_join(concepts, sizeof(concepts), ncos, "Concepts")) return 0;
    nx_make_dir(concepts);
    if (!nx_join(domain_dir, domain_dir_size, concepts, domain_normalized)) return 0;
    nx_make_dir(domain_dir);
    return 1;
}

static void nx_trim_line(char* s)
{
    if (s == NULL) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ' || s[n - 1] == '\t'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

int NxConcept_Normalize(const char* input, char* output, size_t output_size)
{
    if (input == NULL || output == NULL || output_size == 0) return 0;
    size_t j = 0;
    int last_us = 0;
    for (size_t i = 0; input[i] != '\0' && j + 1 < output_size; ++i)
    {
        unsigned char c = (unsigned char)input[i];
        if (isalnum(c))
        {
            output[j++] = (char)tolower(c);
            last_us = 0;
        }
        else if (!last_us && j > 0)
        {
            output[j++] = '_';
            last_us = 1;
        }
    }
    while (j > 0 && output[j - 1] == '_') j--;
    output[j] = '\0';
    return j > 0;
}

int NxConcept_PathFor(const char* root_path,
                      const char* domain,
                      const char* concept_name,
                      char* output,
                      size_t output_size)
{
    if (root_path == NULL || domain == NULL || concept_name == NULL || output == NULL) return 0;
    char ndomain[128];
    char nconcept[128];
    char domain_dir[512];
    char file_name[192];
    if (!NxConcept_Normalize(domain, ndomain, sizeof(ndomain))) return 0;
    if (!NxConcept_Normalize(concept_name, nconcept, sizeof(nconcept))) return 0;
    if (!nx_make_concept_dirs(root_path, ndomain, domain_dir, sizeof(domain_dir))) return 0;
    int written = snprintf(file_name, sizeof(file_name), "%s.card", nconcept);
    if (written < 0 || (size_t)written >= sizeof(file_name)) return 0;
    return nx_join(output, output_size, domain_dir, file_name);
}

static void nx_card_init(NxConceptCard* card)
{
    if (card != NULL) memset(card, 0, sizeof(*card));
}

int NxConcept_Load(const char* root_path,
                   const char* domain,
                   const char* concept_name,
                   NxConceptCard* out_card)
{
    if (out_card == NULL) return 0;
    nx_card_init(out_card);
    char path[512];
    if (!NxConcept_PathFor(root_path, domain, concept_name, path, sizeof(path))) return 0;
    FILE* f = fopen(path, "rb");
    if (f == NULL) return 0;

    nx_copy(out_card->path, sizeof(out_card->path), path);
    nx_copy(out_card->domain, sizeof(out_card->domain), domain);
    nx_copy(out_card->name, sizeof(out_card->name), concept_name);
    NxConcept_Normalize(concept_name, out_card->normalized_name, sizeof(out_card->normalized_name));

    char line[1400];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        nx_trim_line(line);
        char* eq = strchr(line, '=');
        if (eq == NULL) continue;
        *eq = '\0';
        const char* key = line;
        const char* value = eq + 1;
        if (strcmp(key, "domain") == 0) nx_copy(out_card->domain, sizeof(out_card->domain), value);
        else if (strcmp(key, "name") == 0) nx_copy(out_card->name, sizeof(out_card->name), value);
        else if (strcmp(key, "normalized") == 0) nx_copy(out_card->normalized_name, sizeof(out_card->normalized_name), value);
        else if (strcmp(key, "definition") == 0) nx_copy(out_card->definition, sizeof(out_card->definition), value);
        else if (strcmp(key, "purpose") == 0) nx_copy(out_card->purpose, sizeof(out_card->purpose), value);
        else if (strcmp(key, "relationships") == 0) nx_copy(out_card->relationships, sizeof(out_card->relationships), value);
        else if (strcmp(key, "confidence") == 0) out_card->confidence = atof(value);
        else if (strcmp(key, "version") == 0) out_card->version = atoi(value);
    }
    fclose(f);
    if (out_card->version <= 0) out_card->version = 1;
    return 1;
}

int NxConcept_Upsert(const char* root_path,
                     const char* domain,
                     const char* concept_name,
                     const char* definition,
                     const char* purpose,
                     const char* relationships,
                     double confidence,
                     NxConceptCard* out_card)
{
    if (root_path == NULL || domain == NULL || concept_name == NULL) return 0;
    NxConceptCard previous;
    int existed = NxConcept_Load(root_path, domain, concept_name, &previous);

    NxConceptCard card;
    nx_card_init(&card);
    nx_copy(card.domain, sizeof(card.domain), domain);
    nx_copy(card.name, sizeof(card.name), concept_name);
    if (!NxConcept_Normalize(concept_name, card.normalized_name, sizeof(card.normalized_name))) return 0;
    nx_copy(card.definition, sizeof(card.definition), definition && definition[0] ? definition : (existed ? previous.definition : "Sin definicion registrada."));
    nx_copy(card.purpose, sizeof(card.purpose), purpose && purpose[0] ? purpose : (existed ? previous.purpose : "Sin proposito registrado."));
    nx_copy(card.relationships, sizeof(card.relationships), relationships && relationships[0] ? relationships : (existed ? previous.relationships : ""));
    card.confidence = confidence > 0.0 ? confidence : (existed ? previous.confidence : 0.50);
    if (card.confidence > 1.0) card.confidence = 1.0;
    card.version = existed ? previous.version + 1 : 1;

    if (!NxConcept_PathFor(root_path, domain, concept_name, card.path, sizeof(card.path))) return 0;
    FILE* f = fopen(card.path, "wb");
    if (f == NULL) return 0;
    fprintf(f, "domain=%s\n", card.domain);
    fprintf(f, "name=%s\n", card.name);
    fprintf(f, "normalized=%s\n", card.normalized_name);
    fprintf(f, "definition=%s\n", card.definition);
    fprintf(f, "purpose=%s\n", card.purpose);
    fprintf(f, "relationships=%s\n", card.relationships);
    fprintf(f, "confidence=%.2f\n", card.confidence);
    fprintf(f, "version=%d\n", card.version);
    fclose(f);

    if (out_card != NULL) *out_card = card;
    return 1;
}

int NxConcept_FormatAnswer(const NxConceptCard* card, char* output, size_t output_size)
{
    if (card == NULL || output == NULL || output_size == 0) return 0;
    int confidence_pct = (int)(card->confidence * 100.0 + 0.5);
    int written = snprintf(output, output_size,
        "Concepto: %s\n"
        "Dominio: %s\n"
        "Version: %d\n\n"
        "Definicion:\n%s\n\n"
        "Proposito:\n%s\n\n"
        "Relaciones:\n%s\n\n"
        "Confianza: %d %%\n"
        "Fuente: %s\n",
        card->name,
        card->domain,
        card->version,
        card->definition,
        card->purpose,
        card->relationships[0] ? card->relationships : "Sin relaciones registradas.",
        confidence_pct,
        card->path);
    return written > 0 && (size_t)written < output_size;
}
