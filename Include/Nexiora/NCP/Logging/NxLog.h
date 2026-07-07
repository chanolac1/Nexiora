#ifndef NEXIORA_NCP_LOGGING_NXLOG_H
#define NEXIORA_NCP_LOGGING_NXLOG_H

#include "Nexiora/NCP/Common/NxResult.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxLogLevel {
    NX_LOG_TRACE = 0,
    NX_LOG_INFO = 1,
    NX_LOG_WARN = 2,
    NX_LOG_ERROR = 3
} NxLogLevel;

NxResult nx_log_initialize(const char* file_path);
void nx_log_shutdown(void);
void nx_log_write(NxLogLevel level, const char* component, const char* message);

#ifdef __cplusplus
}
#endif

#endif
