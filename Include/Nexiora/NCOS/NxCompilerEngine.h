#ifndef NEXIORA_NCOS_COMPILER_ENGINE_H
#define NEXIORA_NCOS_COMPILER_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum NxCompilerRunStatus
{
    NX_COMPILER_RUN_OK = 0,
    NX_COMPILER_RUN_FAILED = 1,
    NX_COMPILER_RUN_INVALID_ARGUMENT = 2,
    NX_COMPILER_RUN_IO_ERROR = 3
} NxCompilerRunStatus;

typedef struct NxCompilerRunResult
{
    NxCompilerRunStatus status;
    int exit_code;
    char command[512];
    char log_path[512];
    char summary[256];
    size_t bytes_captured;
    int saw_error;
    int saw_warning;
} NxCompilerRunResult;

NxCompilerRunStatus NxCompiler_Run(const char* root,
                                    const char* command,
                                    const char* run_id,
                                    NxCompilerRunResult* out_result);

const char* NxCompiler_StatusText(NxCompilerRunStatus status);

#ifdef __cplusplus
}
#endif

#endif
