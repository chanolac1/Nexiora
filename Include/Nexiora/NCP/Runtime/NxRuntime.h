#ifndef NEXIORA_NCP_RUNTIME_NXRUNTIME_H
#define NEXIORA_NCP_RUNTIME_NXRUNTIME_H

#include "Nexiora/NCP/Common/NxResult.h"
#include "Nexiora/NCP/Hardware/NxHardware.h"
#include "Nexiora/NCP/Memory/NxMemory.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxRuntime {
    NxHardwareInfo hardware;
    NxMemoryStats memory_at_start;
    int initialized;
} NxRuntime;

NxResult nx_runtime_initialize(NxRuntime* runtime, const char* log_path);
void nx_runtime_shutdown(NxRuntime* runtime);
void nx_runtime_print_status(const NxRuntime* runtime);

#ifdef __cplusplus
}
#endif

#endif
