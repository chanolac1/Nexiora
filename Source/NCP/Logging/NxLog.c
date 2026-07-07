#include "Nexiora/NCP/Logging/NxLog.h"
#include <stdio.h>
#include <time.h>

static FILE* g_log_file = NULL;

static const char* nx_log_level_name(NxLogLevel level) {
    switch (level) {
        case NX_LOG_TRACE: return "TRACE";
        case NX_LOG_INFO: return "INFO";
        case NX_LOG_WARN: return "WARN";
        case NX_LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

NxResult nx_log_initialize(const char* file_path) {
    if (!file_path) {
        return NX_ERROR_ARGUMENT;
    }

    g_log_file = fopen(file_path, "a");
    if (!g_log_file) {
        return NX_ERROR_IO;
    }

    nx_log_write(NX_LOG_INFO, "Logging", "Nexiora logging initialized.");
    return NX_OK;
}

void nx_log_shutdown(void) {
    if (g_log_file) {
        nx_log_write(NX_LOG_INFO, "Logging", "Nexiora logging shutdown.");
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

void nx_log_write(NxLogLevel level, const char* component, const char* message) {
    FILE* output = g_log_file ? g_log_file : stderr;
    time_t now = time(NULL);
    struct tm tm_value;
#if defined(_WIN32)
    localtime_s(&tm_value, &now);
#else
    localtime_r(&now, &tm_value);
#endif
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_value);
    fprintf(output, "[%s] [%s] [%s] %s\n", timestamp, nx_log_level_name(level), component ? component : "Core", message ? message : "");
    fflush(output);
}
