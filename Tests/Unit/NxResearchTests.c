#include "Nexiora/Research/NxExperiment.h"
#include "Nexiora/Research/NxResearchKernel.h"
#include "Nexiora/Research/NxManifest.h"
#include "Nexiora/Research/NxRegistry.h"
#include <stdio.h>
#include <string.h>

#define NX_TEST_ASSERT(condition) do { \
    if (!(condition)) { \
        printf("FAILED: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int test_experiment_lifecycle(void) {
    NxExperiment experiment;
    NX_TEST_ASSERT(nx_experiment_initialize(&experiment,
                                            "LAB-0001",
                                            "Scalar String Copy Baseline",
                                            "Jorge Hernandez",
                                            "String",
                                            "Establish the baseline for string copy experiments.",
                                            "Runtime") == NX_OK);
    NX_TEST_ASSERT(strcmp(experiment.id, "LAB-0001") == 0);
    NX_TEST_ASSERT(experiment.status == NX_EXPERIMENT_STATUS_DRAFT);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_PREPARED) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_RUNNING) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_MEASURED) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_APPROVED) == NX_ERROR_UNSUPPORTED);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_VALIDATED) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_transition(&experiment, NX_EXPERIMENT_STATUS_APPROVED) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_hash(&experiment) != 0);
    return 0;
}

static int test_research_kernel_registry(void) {
    NxResearchKernel kernel;
    NxResearchKernelConfig config;
    config.root_path = "Research";
    config.registry_path = "test_registry.nxr";
    config.journal_path = "test_journal.log";

    NX_TEST_ASSERT(nx_research_kernel_initialize(&kernel, &config) == NX_OK);
    NX_TEST_ASSERT(kernel.initialized == 1);

    NxExperiment experiment;
    NX_TEST_ASSERT(nx_experiment_initialize(&experiment,
                                            "LAB-0002",
                                            "Research Kernel Smoke Test",
                                            "Nexiora",
                                            "Research",
                                            "The kernel should register and persist experiment metadata.",
                                            "Research") == NX_OK);
    NX_TEST_ASSERT(nx_research_kernel_create_experiment(&kernel, &experiment) == NX_OK);
    NX_TEST_ASSERT(kernel.experiment_count == 1);
    NX_TEST_ASSERT(nx_research_kernel_write_registry(&kernel) == NX_OK);

    NxResearchKernelStats stats = nx_research_kernel_get_stats(&kernel);
    NX_TEST_ASSERT(stats.created_experiments == 1);
    NX_TEST_ASSERT(stats.registered_experiment_count == 1);
    NX_TEST_ASSERT(stats.journal_events >= 1);

    nx_research_kernel_shutdown(&kernel);
    NX_TEST_ASSERT(kernel.initialized == 0);
    return 0;
}


static int test_manifest_roundtrip(void) {
    NxExperiment original;
    NxExperiment loaded;
    const char* path = "test_experiment_manifest.nx";

    NX_TEST_ASSERT(nx_experiment_initialize(&original,
                                            "LAB-0100",
                                            "Manifest Roundtrip",
                                            "Nexiora",
                                            "Research",
                                            "A manifest written by Nexiora should be readable as an experiment object.",
                                            "Research") == NX_OK);
    NX_TEST_ASSERT(nx_experiment_transition(&original, NX_EXPERIMENT_STATUS_PREPARED) == NX_OK);
    NX_TEST_ASSERT(nx_manifest_write_experiment(path, &original) == NX_OK);
    NX_TEST_ASSERT(nx_manifest_read_experiment(path, &loaded) == NX_OK);
    NX_TEST_ASSERT(strcmp(loaded.id, "LAB-0100") == 0);
    NX_TEST_ASSERT(strcmp(loaded.title, "Manifest Roundtrip") == 0);
    NX_TEST_ASSERT(loaded.status == NX_EXPERIMENT_STATUS_PREPARED);
    return 0;
}

static int test_registry_roundtrip(void) {
    NxRegistry registry;
    NxRegistry loaded;
    NxExperiment experiment;
    const char* path = "test_registry_roundtrip.nxr";

    NX_TEST_ASSERT(nx_registry_initialize(&registry) == NX_OK);
    NX_TEST_ASSERT(nx_experiment_initialize(&experiment,
                                            "LAB-0101",
                                            "Registry Roundtrip",
                                            "Nexiora",
                                            "Research",
                                            "A registry should persist experiment index records.",
                                            "Research") == NX_OK);
    NX_TEST_ASSERT(nx_registry_add(&registry, &experiment) == NX_OK);
    NX_TEST_ASSERT(nx_registry_write(&registry, path) == NX_OK);
    NX_TEST_ASSERT(nx_registry_read(&loaded, path) == NX_OK);
    NX_TEST_ASSERT(loaded.count == 1);
    NX_TEST_ASSERT(nx_registry_find(&loaded, "LAB-0101") != NULL);
    NX_TEST_ASSERT(nx_registry_find(&loaded, "LAB-DOES-NOT-EXIST") == NULL);
    return 0;
}

int main(void) {
    int failed = 0;
    failed += test_experiment_lifecycle();
    failed += test_research_kernel_registry();
    failed += test_manifest_roundtrip();
    failed += test_registry_roundtrip();

    if (failed == 0) {
        printf("Nexiora Research Tests: PASS\n");
        return 0;
    }

    printf("Nexiora Research Tests: FAIL\n");
    return 1;
}
