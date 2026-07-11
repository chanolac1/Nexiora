#include "Nexiora/Memory/NxVersionedSemanticMemory.h"

#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#define nx_test_pid() ((unsigned int)_getpid())
#else
#include <unistd.h>
#define nx_test_pid() ((unsigned int)getpid())
#endif

static int nx_check(int condition, const char* message) {
    if (!condition) {
        (void)fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    char path[256];
    NxVsmRememberResult first;
    NxVsmRememberResult second;
    NxVsmRememberResult unchanged;
    NxVsmEntry recalled;
    NxVsmEntry history[4];
    size_t count;
    int ok = 1;
    (void)snprintf(path, sizeof(path), "Build/vsm_test_%u/memory.nxsemantic", nx_test_pid());
    ok &= nx_check(NxVersionedSemanticMemory_Init(path) == NX_VSM_OK, "init failed");
    ok &= nx_check(NxVersionedSemanticMemory_Remember(path, "consolidacion de memoria",
        "El sueno es el unico mecanismo de consolidacion.", "fuente-a", "2026-07-11T10:00:00Z", 55U, &first) == NX_VSM_OK,
        "first belief failed");
    ok &= nx_check(first.version == 1U && first.previous_entry_id == 0U, "first version metadata invalid");
    ok &= nx_check(NxVersionedSemanticMemory_Remember(path, "consolidacion de memoria",
        "La consolidacion es multifactorial: sueno, repeticion y recuperacion activa.", "fuente-b", "2026-07-11T11:00:00Z", 88U, &second) == NX_VSM_OK,
        "belief update failed");
    ok &= nx_check(second.version == 2U && second.previous_entry_id == first.entry_id, "version chain invalid");
    ok &= nx_check(second.belief_changed != 0, "belief change was not detected");
    ok &= nx_check(strstr(second.previous_belief, "unico") != NULL, "previous belief not preserved");
    ok &= nx_check(NxVersionedSemanticMemory_Recall(path, "consolidacion de memoria", &recalled) == NX_VSM_OK,
        "recall failed");
    ok &= nx_check(recalled.version == 2U && recalled.active != 0, "active version incorrect");
    ok &= nx_check(strstr(recalled.belief, "multifactorial") != NULL, "updated belief not active");
    count = NxVersionedSemanticMemory_History(path, "consolidacion de memoria", history, 4U);
    ok &= nx_check(count == 2U, "history count incorrect");
    ok &= nx_check(history[0].active == 0 && history[1].active != 0, "history active states incorrect");
    ok &= nx_check(NxVersionedSemanticMemory_Remember(path, "consolidacion de memoria",
        "La consolidacion es multifactorial: sueno, repeticion y recuperacion activa.", "fuente-b", "2026-07-11T12:00:00Z", 80U, &unchanged) == NX_VSM_UNCHANGED,
        "redundant lower-confidence belief should be unchanged");
    if (!ok) return 1;
    (void)printf("NxVersionedSemanticMemoryTests: PASS\n");
    return 0;
}
