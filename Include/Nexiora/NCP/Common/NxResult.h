#ifndef NEXIORA_NCP_COMMON_NXRESULT_H
#define NEXIORA_NCP_COMMON_NXRESULT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxResult {
    NX_OK = 0,
    NX_ERROR_ARGUMENT = 1,
    NX_ERROR_MEMORY = 2,
    NX_ERROR_IO = 3,
    NX_ERROR_UNSUPPORTED = 4,
    NX_ERROR_INTERNAL = 5
} NxResult;

const char* nx_result_to_string(NxResult result);

#ifdef __cplusplus
}
#endif

#endif
