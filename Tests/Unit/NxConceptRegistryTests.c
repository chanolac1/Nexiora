#include "Nexiora/NCOS/NxConceptRegistry.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void expect_true(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

int main(void)
{
    const char* root = ".nexiora_test_ncos003_registry";
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

    if (failures != 0)
    {
        return 1;
    }
    return 0;
}
