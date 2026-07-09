#include "Nexiora/NCOS/NxAnswerComposer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

static int fails = 0;
#define CHECK(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); ++fails; } } while (0)

static void write_text(const char* path, const char* text)
{
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fputs(text, f);
    fclose(f);
}

int main(void)
{
    NxAnswerComposerResult result;
    system("rmdir /S /Q .ncos_answer_test >NUL 2>NUL");
    MKDIR(".ncos_answer_test");
    MKDIR(".ncos_answer_test/Knowledge");
    MKDIR(".ncos_answer_test/Knowledge/NCOS");
    MKDIR(".ncos_answer_test/Knowledge/NCOS/Concepts");
    MKDIR(".ncos_answer_test/Knowledge/NCOS/Concepts/genexus");
    MKDIR(".ncos_answer_test/Knowledge/NCOS/ConceptGraphs");
    MKDIR(".ncos_answer_test/Knowledge/NCOS/ConceptGraphs/genexus");

    write_text(".ncos_answer_test/Knowledge/NCOS/Concepts/genexus/dataselector.card",
               "name: DataSelector\nversion: 1\ndefinition: Un DataSelector define una consulta reutilizable sobre datos.\npurpose: Sirve para evitar repetir filtros y reglas de seleccion.\nrelations: DataProvider,Procedure\nconfidence: 85\n");
    write_text(".ncos_answer_test/Knowledge/NCOS/Concepts/genexus/dataprovider.card",
               "name: DataProvider\nversion: 1\ndefinition: Un Data Provider construye y devuelve estructuras de datos.\npurpose: Sirve para preparar datos consumibles por otros objetos.\nrelations: KnowledgeBase\nconfidence: 85\n");
    write_text(".ncos_answer_test/Knowledge/NCOS/Concepts/genexus/knowledgebase.card",
               "name: KnowledgeBase\nversion: 1\ndefinition: La Knowledge Base es el repositorio central del conocimiento.\npurpose: Sirve para generar aplicaciones desde el modelo.\nrelations: Transaction,Procedure\nconfidence: 85\n");
    write_text(".ncos_answer_test/Knowledge/NCOS/ConceptGraphs/genexus/edges.jsonl",
               "dataselector|related_to|dataprovider|85\ndataprovider|belongs_to|knowledgebase|85\n");

    CHECK(NxAnswerComposer_ComposeRelationAnswer(".ncos_answer_test", "Genexus", "DataSelector", "KnowledgeBase", &result) == 1, "compose should succeed");
    CHECK(strstr(result.answer, "Respuesta estructurada") != NULL, "answer should be structured");
    CHECK(strstr(result.answer, "DataSelector") != NULL, "answer should mention source concept");
    CHECK(strstr(result.answer, "KnowledgeBase") != NULL, "answer should mention target concept");
    CHECK(strstr(result.answer, "dataprovider") != NULL || strstr(result.answer, "DataProvider") != NULL, "answer should mention intermediate concept");
    CHECK(result.confidence >= 70, "confidence should be meaningful");
    CHECK(result.evidence_count >= 2, "answer should expose evidence");
    CHECK(NxAnswerComposer_WriteAnswer(".ncos_answer_test/answer.txt", &result) == 1, "write answer should succeed");

    return fails == 0 ? 0 : 1;
}
