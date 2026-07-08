#include "Nexiora/Research/NxDiscoveryEngine.h"

#include <stdio.h>

#define NX_DISCOVERY_BENCH_CAPACITY 128
#define NX_DISCOVERY_BENCH_ITERATIONS 10000

int main(void)
{
    NxExperimentDescriptor storage[NX_DISCOVERY_BENCH_CAPACITY];
    NxDiscoveryEngine engine;
    NxExperimentDescriptor descriptor;
    const NxExperimentDescriptor* results[NX_DISCOVERY_BENCH_CAPACITY];
    size_t index = 0;
    size_t iteration = 0;
    size_t total_matches = 0;

    if (NxDiscoveryEngine_Init(&engine, storage, NX_DISCOVERY_BENCH_CAPACITY) != NX_DISCOVERY_STATUS_OK)
    {
        return 1;
    }

    for (index = 0; index < NX_DISCOVERY_BENCH_CAPACITY; ++index)
    {
        static char ids[NX_DISCOVERY_BENCH_CAPACITY][32];
        static char names[NX_DISCOVERY_BENCH_CAPACITY][32];

        (void)snprintf(ids[index], sizeof(ids[index]), "bench.%03u", (unsigned int)index);
        (void)snprintf(names[index], sizeof(names[index]), "Benchmark %03u", (unsigned int)index);

        descriptor.id = ids[index];
        descriptor.name = names[index];
        descriptor.component = (index % 2 == 0) ? "Vector" : "String";
        descriptor.owner = "benchmark";
        descriptor.kind = NX_EXPERIMENT_KIND_BENCHMARK;
        descriptor.state = (index % 3 == 0) ? NX_EXPERIMENT_STATE_DISABLED : NX_EXPERIMENT_STATE_READY;
        descriptor.priority = (unsigned int)(index % 10);

        if (NxDiscoveryEngine_Register(&engine, &descriptor) != NX_DISCOVERY_STATUS_OK)
        {
            return 1;
        }
    }

    for (iteration = 0; iteration < NX_DISCOVERY_BENCH_ITERATIONS; ++iteration)
    {
        total_matches += NxDiscoveryEngine_Discover(
            &engine,
            NxDiscoveryPredicate_Ready,
            0,
            results,
            NX_DISCOVERY_BENCH_CAPACITY);
    }

    printf("NxDiscoveryEngineBenchmark: iterations=%u experiments=%u total_matches=%u\n",
        (unsigned int)NX_DISCOVERY_BENCH_ITERATIONS,
        (unsigned int)NX_DISCOVERY_BENCH_CAPACITY,
        (unsigned int)total_matches);

    return total_matches > 0 ? 0 : 1;
}
