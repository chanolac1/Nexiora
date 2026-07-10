#include "Nexiora/NCOS/NxDocumentationGovernance.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#define NX_MKDIR(path) mkdir((path), 0777)
#endif
static int failures = 0;
static void expect(int condition, const char* message) { if (!condition) { printf("FAIL: %s\n", message); failures++; } }
static void make_dir(const char* path) { (void)NX_MKDIR(path); }
static void write_text(const char* path, const char* text) { FILE* f = fopen(path, "wb"); if (f != NULL) { (void)fputs(text, f); (void)fclose(f); } }
int main(void)
{
    const char* root = "Build/docs_governance_test";
    const char* names[] = {"MASTER_CONTEXT.md","PROJECT_STATE.md","ROADMAP.md","CHANGELOG.md","DECISIONS.md","ARCHITECTURE.md","CODING_STANDARD.md","TESTING_STANDARD.md","PACKAGE_STANDARD.md"};
    size_t i;
    NxDocumentationValidationResult result;
    char path[512];
    make_dir("Build"); make_dir(root); (void)snprintf(path, sizeof(path), "%s/Docs", root); make_dir(path);
    for (i = 0U; i < sizeof(names)/sizeof(names[0]); ++i) { const char* content = "# DOCUMENT\n"; (void)snprintf(path, sizeof(path), "%s/Docs/%s", root, names[i]); if (strcmp(names[i], "PROJECT_STATE.md") == 0) content="# PROJECT STATE\n"; else if (strcmp(names[i], "CHANGELOG.md") == 0) content="# CHANGELOG\n"; else if (strcmp(names[i], "DECISIONS.md") == 0) content="# DECISIONS\n"; write_text(path, content); }
    expect(NxDocumentation_Validate(root, &result) == 1, "complete documentation set should validate");
    expect(result.files_checked == 9, "nine documents should be checked");
    expect(NxDocumentation_CompleteSprint(root, "Sprint 20", "Self Validation Engine", "2026-07-10") == 1, "sprint completion should update documents");
    expect(NxDocumentation_RecordDecision(root, "ADR-TEST", "Test decision", "Decision body.", "2026-07-10") == 1, "decision should be recorded");
    (void)snprintf(path, sizeof(path), "%s/Docs/PACKAGE_STANDARD.md", root); (void)remove(path);
    expect(NxDocumentation_Validate(root, &result) == 0, "missing document should fail validation");
    expect(result.files_missing == 1, "one missing document should be reported");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
