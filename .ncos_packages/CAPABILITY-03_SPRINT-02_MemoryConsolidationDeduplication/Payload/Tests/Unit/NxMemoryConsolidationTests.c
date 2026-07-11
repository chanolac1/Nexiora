#include "Nexiora/Memory/NxMemoryConsolidation.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#define nx_test_pid() ((unsigned int)_getpid())
#else
#include <unistd.h>
#define nx_test_pid() ((unsigned int)getpid())
#endif

static int failures = 0;
#define CHECK(condition, message) do { if (!(condition)) { printf("FAIL: %s\n", message); failures++; } } while (0)

static void make_path(char* out, size_t capacity, const char* suffix) {
    (void)snprintf(out, capacity, "Build/nx_mc_%u/%s", nx_test_pid(), suffix);
}

int main(void) {
    char memory_path[1024];
    NxVsmRememberResult remembered;
    NxMcReport report;
    NxVsmEntry active;
    NxMcStatus status;
    make_path(memory_path, sizeof(memory_path), "semantic.nxsemantic");

    CHECK(NxVersionedSemanticMemory_Init(memory_path) == NX_VSM_OK, "memory initialization failed");
    CHECK(NxVersionedSemanticMemory_Remember(memory_path, "memoria episodica",
        "La memoria episodica conserva experiencias personales con contexto autobiografico.",
        "fuente-a", "2026-07-11T10:00:00Z", 60U, &remembered) == NX_VSM_OK,
        "first observation failed");
    CHECK(NxVersionedSemanticMemory_Remember(memory_path, "memoria episodica",
        "La memoria episodica conserva experiencias personales con contexto autobiografico.",
        "fuente-b", "2026-07-11T11:00:00Z", 70U, &remembered) == NX_VSM_OK,
        "second independent observation failed");

    status = NxMemoryConsolidation_Analyze(memory_path, "memoria episodica", 65U, &report);
    CHECK(status == NX_MC_OK, "independent equivalent evidence should be consolidatable");
    CHECK(report.independent_sources == 2U, "independent source count should be two");
    CHECK(report.supporting_entries == 2U, "supporting entry count should be two");
    CHECK(report.proposed_confidence > report.old_confidence, "confidence should increase after independent support");

    status = NxMemoryConsolidation_Consolidate(memory_path, "memoria episodica",
        "2026-07-11T12:00:00Z", 65U, &report);
    CHECK(status == NX_MC_OK, "consolidation should create a versioned reinforcement");
    CHECK(NxVersionedSemanticMemory_Recall(memory_path, "memoria episodica", &active) == NX_VSM_OK,
        "consolidated belief should be recallable");
    CHECK(active.version == 3U, "consolidation should create version three");
    CHECK(active.confidence == report.proposed_confidence, "stored confidence should match proposal");

    CHECK(NxVersionedSemanticMemory_Remember(memory_path, "memoria episodica",
        "La memoria episodica no conserva experiencias personales con contexto autobiografico.",
        "fuente-c", "2026-07-11T13:00:00Z", 75U, &remembered) == NX_VSM_OK,
        "opposing observation failed");
    status = NxMemoryConsolidation_Analyze(memory_path, "memoria episodica", 65U, &report);
    CHECK(status == NX_MC_CONFLICT_DETECTED, "negated equivalent proposition should be detected as conflict");
    CHECK(report.opposing_entries >= 1U, "conflict report should expose opposing entries");

    CHECK(NxMemoryConsolidation_Similarity("memoria episodica experiencias personales",
        "experiencias personales memoria episodica") == 100U,
        "token similarity should be order independent");

    if (failures == 0) {
        puts("NxMemoryConsolidationTests: PASS");
        return 0;
    }
    return 1;
}
