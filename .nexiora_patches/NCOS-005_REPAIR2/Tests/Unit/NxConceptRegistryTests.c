#include "Nexiora/NCOS/NxConceptRegistry.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_RMDIR(path) _rmdir(path)
#else
#include <unistd.h>
#define NX_RMDIR(path) rmdir(path)
#endif

static int failures = 0;

static void expect_true(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static void nx_join_test(char* dst, size_t dst_size, const char* a, const char* b)
{
    const char* sep = "";
    size_t n = strlen(a);
    if (n > 0 && a[n - 1] != '/' && a[n - 1] != '\\') sep = "/";
    (void)snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    dst[dst_size - 1] = '\0';
}

static void cleanup_registry_test_root(const char* root)
{
    char p1[512], p2[512], p3[512], p4[512], p5[512], p6[512];
    nx_join_test(p1, sizeof(p1), root, "Knowledge");
    nx_join_test(p2, sizeof(p2), p1, "NCOS");
    nx_join_test(p3, sizeof(p3), p2, "Concepts");
    nx_join_test(p4, sizeof(p4), p3, "genexus");
    nx_join_test(p5, sizeof(p5), p4, "dataselector.card");
    nx_join_test(p6, sizeof(p6), p4, "knowledge_base.card");

    (void)remove(p5);
    (void)remove(p6);
    (void)NX_RMDIR(p4);
    (void)NX_RMDIR(p3);
    (void)NX_RMDIR(p2);
    (void)NX_RMDIR(p1);
    (void)NX_RMDIR(root);
}

int main(void)
{
    const char* root = ".nexiora_test_ncos003_registry";
    cleanup_registry_test_root(root);

    char normalized[128];
    expect_true(NxConcept_Normalize("Knowledge Base", normalized, sizeof(normalized)), "normalize should succeed");
    expect_true(strcmp(normalized, "knowledge_base") == 0, "normalize should use stable concept identity");

    NxConceptCard card;
    expect_true(NxConcept_Upsert(root,
                                 "Genexus",
                                 "DataSelector",
                                 "Un DataSelector define una consulta reutilizable sobre datos.",
                                 "Sirve para evitar repetir filtros y reglas de seleccion.",
                                 "Data Provider,Transaction,Procedure",
                                 0.86,
                                 &card),
                "upsert should succeed");
    expect_true(strcmp(card.normalized_name, "dataselector") == 0, "concept should expose normalized name");
    expect_true(card.version == 1, "first upsert should create version 1");
    expect_true(strstr(card.path, "dataselector.card") != NULL, "concept should expose card path");

    NxConceptCard loaded;
    expect_true(NxConcept_Load(root, "Genexus", "DataSelector", &loaded), "load should succeed");
    expect_true(strstr(loaded.definition, "consulta reutilizable") != NULL, "load should preserve definition");
    expect_true(strstr(loaded.relationships, "Data Provider") != NULL, "load should preserve relationships");

    NxConceptCard updated;
    expect_true(NxConcept_Upsert(root,
                                 "Genexus",
                                 "DataSelector",
                                 "Un DataSelector encapsula una consulta declarativa reutilizable.",
                                 "Ayuda a centralizar criterios de seleccion.",
                                 "Data Provider,Transaction,Knowledge Base",
                                 0.91,
                                 &updated),
                "second upsert should succeed");
    expect_true(updated.version == 2, "second upsert should evolve concept version");
    expect_true(updated.confidence > loaded.confidence, "confidence should improve when better evidence arrives");

    char answer[4096];
    expect_true(NxConcept_FormatAnswer(&updated, answer, sizeof(answer)), "format answer should succeed");
    expect_true(strstr(answer, "Definicion") != NULL, "answer should be structured");
    expect_true(strstr(answer, "Proposito") != NULL, "answer should include purpose");
    expect_true(strstr(answer, "Relaciones") != NULL, "answer should include relations");

    cleanup_registry_test_root(root);

    if (failures != 0)
    {
        return 1;
    }
    return 0;
}
