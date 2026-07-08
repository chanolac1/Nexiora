#include "Nexiora/Research/NxDiscoveryEngine.h"

#include <stdio.h>

static int Expect(int condition, const char* message)
{
    if (!condition)
    {
        printf("FAILED: %s\n", message);
        return 1;
    }

    return 0;
}

static NxExperimentDescriptor MakeDescriptor(
    const char* id,
    const char* name,
    const char* component,
    NxExperimentKind kind,
    NxExperimentState state,
    unsigned int priority)
{
    NxExperimentDescriptor descriptor;

    descriptor.id = id;
    descriptor.name = name;
    descriptor.component = component;
    descriptor.owner = "research";
    descriptor.kind = kind;
    descriptor.state = state;
    descriptor.priority = priority;

    return descriptor;
}

static int TestRegisterAndCount(void)
{
    NxExperimentDescriptor storage[4];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor descriptor;

    if (Expect(NxDiscoveryEngine_Init(&engine, storage, 4) == NX_DISCOVERY_STATUS_OK,
        "init should succeed")) return 1;

    descriptor = MakeDescriptor(
        "vector.simd.001",
        "SIMD Vector Experiment",
        "Vector",
        NX_EXPERIMENT_KIND_BENCHMARK,
        NX_EXPERIMENT_STATE_READY,
        10);

    if (Expect(NxDiscoveryEngine_Register(&engine, &descriptor) == NX_DISCOVERY_STATUS_OK,
        "register should succeed")) return 1;

    if (Expect(NxDiscoveryEngine_Count(&engine) == 1,
        "count should be 1")) return 1;

    return 0;
}

static int TestDuplicateIdRejected(void)
{
    NxExperimentDescriptor storage[4];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor first;
    NxExperimentDescriptor duplicate;

    NxDiscoveryEngine_Init(&engine, storage, 4);

    first = MakeDescriptor("same.id", "First", "Core", NX_EXPERIMENT_KIND_RESEARCH, NX_EXPERIMENT_STATE_READY, 1);
    duplicate = MakeDescriptor("same.id", "Duplicate", "Core", NX_EXPERIMENT_KIND_RESEARCH, NX_EXPERIMENT_STATE_READY, 1);

    if (Expect(NxDiscoveryEngine_Register(&engine, &first) == NX_DISCOVERY_STATUS_OK,
        "first registration should succeed")) return 1;

    if (Expect(NxDiscoveryEngine_Register(&engine, &duplicate) == NX_DISCOVERY_STATUS_DUPLICATE_ID,
        "duplicate id should be rejected")) return 1;

    if (Expect(NxDiscoveryEngine_Count(&engine) == 1,
        "duplicate should not increase count")) return 1;

    return 0;
}

static int TestCapacityExceeded(void)
{
    NxExperimentDescriptor storage[1];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor first;
    NxExperimentDescriptor second;

    NxDiscoveryEngine_Init(&engine, storage, 1);

    first = MakeDescriptor("a", "A", "Core", NX_EXPERIMENT_KIND_RESEARCH, NX_EXPERIMENT_STATE_READY, 1);
    second = MakeDescriptor("b", "B", "Core", NX_EXPERIMENT_KIND_RESEARCH, NX_EXPERIMENT_STATE_READY, 1);

    if (Expect(NxDiscoveryEngine_Register(&engine, &first) == NX_DISCOVERY_STATUS_OK,
        "first registration should succeed")) return 1;

    if (Expect(NxDiscoveryEngine_Register(&engine, &second) == NX_DISCOVERY_STATUS_CAPACITY_EXCEEDED,
        "capacity should be enforced")) return 1;

    return 0;
}

static int TestGetAndFindById(void)
{
    NxExperimentDescriptor storage[4];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor descriptor;
    const NxExperimentDescriptor* found = 0;

    NxDiscoveryEngine_Init(&engine, storage, 4);

    descriptor = MakeDescriptor("manifest.001", "Manifest Test", "Manifest", NX_EXPERIMENT_KIND_CORRECTNESS, NX_EXPERIMENT_STATE_READY, 3);
    NxDiscoveryEngine_Register(&engine, &descriptor);

    if (Expect(NxDiscoveryEngine_Get(&engine, 0, &found) == NX_DISCOVERY_STATUS_OK,
        "get index 0 should succeed")) return 1;

    if (Expect(found != 0 && found->priority == 3,
        "get should return descriptor")) return 1;

    found = 0;

    if (Expect(NxDiscoveryEngine_FindById(&engine, "manifest.001", &found) == NX_DISCOVERY_STATUS_OK,
        "find by id should succeed")) return 1;

    if (Expect(found != 0 && found->kind == NX_EXPERIMENT_KIND_CORRECTNESS,
        "find should return descriptor")) return 1;

    if (Expect(NxDiscoveryEngine_FindById(&engine, "missing", &found) == NX_DISCOVERY_STATUS_NOT_FOUND,
        "missing id should return not found")) return 1;

    return 0;
}

static int TestDiscoverReadyAndComponent(void)
{
    NxExperimentDescriptor storage[6];
    const NxExperimentDescriptor* results[6];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor a;
    NxExperimentDescriptor b;
    NxExperimentDescriptor c;
    size_t matched = 0;

    NxDiscoveryEngine_Init(&engine, storage, 6);

    a = MakeDescriptor("vector.ready", "Vector Ready", "Vector", NX_EXPERIMENT_KIND_BENCHMARK, NX_EXPERIMENT_STATE_READY, 10);
    b = MakeDescriptor("vector.disabled", "Vector Disabled", "Vector", NX_EXPERIMENT_KIND_BENCHMARK, NX_EXPERIMENT_STATE_DISABLED, 5);
    c = MakeDescriptor("string.ready", "String Ready", "String", NX_EXPERIMENT_KIND_CORRECTNESS, NX_EXPERIMENT_STATE_READY, 7);

    NxDiscoveryEngine_Register(&engine, &a);
    NxDiscoveryEngine_Register(&engine, &b);
    NxDiscoveryEngine_Register(&engine, &c);

    matched = NxDiscoveryEngine_Discover(&engine, NxDiscoveryPredicate_Ready, 0, results, 6);

    if (Expect(matched == 2,
        "ready discovery should find two experiments")) return 1;

    if (Expect(results[0]->state == NX_EXPERIMENT_STATE_READY && results[1]->state == NX_EXPERIMENT_STATE_READY,
        "ready discovery should only return ready descriptors")) return 1;

    matched = NxDiscoveryEngine_Discover(&engine, NxDiscoveryPredicate_Component, "Vector", results, 6);

    if (Expect(matched == 2,
        "component discovery should find two Vector experiments")) return 1;

    matched = NxDiscoveryEngine_Discover(&engine, NxDiscoveryPredicate_All, 0, results, 2);

    if (Expect(matched == 3,
        "discover should return total matches even when result buffer is smaller")) return 1;

    if (Expect(results[0] != 0 && results[1] != 0,
        "small result buffer should still be populated up to capacity")) return 1;

    return 0;
}

static int TestReset(void)
{
    NxExperimentDescriptor storage[2];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor descriptor;

    NxDiscoveryEngine_Init(&engine, storage, 2);
    descriptor = MakeDescriptor("reset.001", "Reset", "Core", NX_EXPERIMENT_KIND_RESEARCH, NX_EXPERIMENT_STATE_READY, 1);
    NxDiscoveryEngine_Register(&engine, &descriptor);

    NxDiscoveryEngine_Reset(&engine);

    if (Expect(NxDiscoveryEngine_Count(&engine) == 0,
        "reset should clear registered descriptors")) return 1;

    return 0;
}

int main(void)
{
    if (TestRegisterAndCount()) return 1;
    if (TestDuplicateIdRejected()) return 1;
    if (TestCapacityExceeded()) return 1;
    if (TestGetAndFindById()) return 1;
    if (TestDiscoverReadyAndComponent()) return 1;
    if (TestReset()) return 1;

    printf("NxDiscoveryEngineTests passed\n");
    return 0;
}
