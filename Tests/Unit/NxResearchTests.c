#include "Nexiora/Research/NxExperiment.h"
#include "Nexiora/Research/NxResearchKernel.h"
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

int main(void) {
    int failed = 0;
    failed += test_experiment_lifecycle();
    failed += test_research_kernel_registry();

    if (failed == 0) {
        printf("Nexiora Research Tests: PASS\n");
        return 0;
    }

    printf("Nexiora Research Tests: FAIL\n");
    return 1;
}
