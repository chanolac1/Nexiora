#ifndef NEXIORA_RESEARCH_NXMANIFEST_H
#define NEXIORA_RESEARCH_NXMANIFEST_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/Research/NxExperiment.h"

#ifdef __cplusplus
extern "C" {
#endif

NxResult nx_manifest_read_experiment(const char* path, NxExperiment* experiment);
NxResult nx_manifest_write_experiment(const char* path, const NxExperiment* experiment);

#ifdef __cplusplus
}
#endif

#endif
