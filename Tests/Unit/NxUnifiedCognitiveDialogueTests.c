#include "Nexiora/Reasoning/NxUnifiedCognitiveDialogue.h"

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

#ifdef _WIN32
#include <process.h>
#define NX_GETPID() ((unsigned int)_getpid())
#else
#include <unistd.h>
#define NX_GETPID() ((unsigned int)getpid())
#endif

static int failures = 0;
#define CHECK(expr, message) do { if (!(expr)) { fprintf(stderr, "FAIL: %s\n", message); ++failures; } } while (0)

static int write_evidence(const char* path) {
    FILE* f = fopen(path, "wb");
    if (f == NULL) return 0;
    fputs("nxevidence/1\n", f);
    fputs("chunk=1\nsource=demo@00:01\ntext=La memoria episodica conserva experiencias personales con tiempo y contexto autobiografico.\nend_chunk\n", f);
    fputs("chunk=2\nsource=demo@00:02\ntext=La memoria semantica conserva hechos y conceptos generales sin contexto autobiografico.\nend_chunk\n", f);
    fputs("chunk=3\nsource=demo@00:03\ntext=La consolidacion depende principalmente del sueno profundo.\nend_chunk\n", f);
    fputs("chunk=4\nsource=demo@00:04\ntext=La consolidacion no depende principalmente del sueno; la recuperacion activa tambien contribuye.\nend_chunk\n", f);
    return fclose(f) == 0;
}

int main(void) {
    char evidence[256];
    char session[256];
    NxUcdResult result;
    NxUcdStatus status;
    unsigned int pid = NX_GETPID();
    (void)NX_MKDIR("Build");
    (void)snprintf(evidence, sizeof(evidence), "Build/ucd_%u.nxevidence", pid);
    (void)snprintf(session, sizeof(session), "Build/ucd_sessions_%u/demo.nxconversation", pid);
    CHECK(write_evidence(evidence), "evidence should be created");
    status = NxUnifiedCognitiveDialogue_Create(session, evidence, "memoria episodica");
    CHECK(status == NX_UCD_OK, "session should be created");
    status = NxUnifiedCognitiveDialogue_Ask(session, "Que conserva la memoria episodica", &result);
    CHECK(status == NX_UCD_OK, "grounded question should be answered");
    CHECK(result.evidence_count >= 1U, "answer should use evidence");
    CHECK(strstr(result.answer, "experiencias personales") != NULL, "answer should be relevant");
    status = NxUnifiedCognitiveDialogue_Ask(session, "Como funciona un reactor de fusion", &result);
    CHECK(status == NX_UCD_GAP_OPENED, "unsupported question should open a gap");
    CHECK(result.gap_plan_path[0] != '\0', "gap path should be reported");
    status = NxUnifiedCognitiveDialogue_EvaluateClaim(session,
        "La consolidacion depende principalmente del sueno", &result);
    CHECK(status == NX_UCD_CONTRADICTORY, "conflicting claim should remain contradictory");
    CHECK(result.contradiction_count >= 1U, "contradiction should be counted");
    (void)remove(evidence);
    (void)remove(session);
    if (failures != 0) return 1;
    puts("NxUnifiedCognitiveDialogueTests: PASS");
    return 0;
}
