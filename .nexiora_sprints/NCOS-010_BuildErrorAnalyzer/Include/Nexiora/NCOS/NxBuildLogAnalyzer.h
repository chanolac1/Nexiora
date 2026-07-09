#ifndef NEXIORA_NCOS_NX_BUILD_LOG_ANALYZER_H
#define NEXIORA_NCOS_NX_BUILD_LOG_ANALYZER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NxBuildLogFinding {
    char type[32];
    char file[256];
    int line;
    char message[512];
} NxBuildLogFinding;

typedef struct NxBuildLogAnalysis {
    int success;
    int error_count;
    int warning_count;
    int failure_count;
    int finding_count;
    NxBuildLogFinding findings[32];
    char summary[1024];
    char report_path[512];
} NxBuildLogAnalysis;

int NxBuildLogAnalyzer_AnalyzeFile(const char* knowledge_root,
                                   const char* run_id,
                                   const char* log_path,
                                   NxBuildLogAnalysis* out);

#ifdef __cplusplus
}
#endif

#endif
