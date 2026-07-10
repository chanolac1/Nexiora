#include "Nexiora/Multimodal/NxMultimodalCognitiveIntegration.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static int check(int condition, const char* message)
{
    if (condition == 0) {
        fprintf(stderr, "NxMultimodalCognitiveIntegrationTests: FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static int write_text(const char* path, const char* text)
{
    FILE* file = fopen(path, "wb");
    size_t n = strlen(text);
    if (file == NULL) return 0;
    if (fwrite(text, 1U, n, file) != n) { fclose(file); return 0; }
    return fclose(file) == 0 ? 1 : 0;
}

int main(void)
{
    const char* root = "nx_mmci_test";
    const char* knowledge = "nx_mmci_test/sample.nxknowledge";
    NxMmciAnalysisResult analysis;
    NxMmciQueryResult query;
    NxMmciStatus status;
    (void)system("rm -rf nx_mmci_test >/dev/null 2>&1");
#if defined(_WIN32)
    (void)system("if exist nx_mmci_test rmdir /s /q nx_mmci_test >nul 2>&1");
#endif
    if (!check(NX_MKDIR(root) == 0, "test directory could not be created")) return 1;
    if (!check(write_text(knowledge,
        "format=nxknowledge/1\ntitle=Memoria episodica\nmedia_type=text\nsource=demo.txt\n"
        "content_begin\nLa memoria episodica conserva experiencias personales. "
        "La memoria semantica conserva hechos y conceptos. "
        "La consolidacion conecta memoria episodica y aprendizaje.\ncontent_end\n"),
        "knowledge fixture could not be written")) return 1;
    status = NxMmci_Analyze(root, knowledge, "memoria", &analysis);
    if (!check(status == NX_MMCI_OK, "analysis did not succeed")) return 1;
    if (!check(analysis.chunks_written >= 3U, "expected at least three chunks")) return 1;
    if (!check(analysis.concepts_written > 0U, "no concepts were integrated")) return 1;
    status = NxMmci_Query(root, "nx_mmci_test_sample_nxknowledge", "Que conserva la memoria episodica", &query);
    if (!check(status == NX_MMCI_OK, "query did not find evidence")) return 1;
    if (!check(strstr(query.answer, "experiencias personales") != NULL, "answer was not grounded in expected evidence")) return 1;
    puts("NxMultimodalCognitiveIntegrationTests: PASS");
    return 0;
}
