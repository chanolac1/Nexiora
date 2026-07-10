#ifndef NEXIORA_MULTIMODAL_NX_MULTIMODAL_COGNITIVE_INTEGRATION_H
#define NEXIORA_MULTIMODAL_NX_MULTIMODAL_COGNITIVE_INTEGRATION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_MMCI_MAX_PATH 512U
#define NX_MMCI_MAX_MESSAGE 256U
#define NX_MMCI_MAX_ANSWER 4096U

typedef enum NxMmciStatus {
    NX_MMCI_OK = 0,
    NX_MMCI_INVALID_ARGUMENT = 1,
    NX_MMCI_NOT_FOUND = 2,
    NX_MMCI_FORMAT_ERROR = 3,
    NX_MMCI_IO_ERROR = 4,
    NX_MMCI_NO_EVIDENCE = 5,
    NX_MMCI_OUTPUT_TOO_SMALL = 6
} NxMmciStatus;

typedef struct NxMmciAnalysisResult {
    NxMmciStatus status;
    unsigned int chunks_written;
    unsigned int concepts_written;
    unsigned int relations_written;
    unsigned int confidence;
    char analysis_dir[NX_MMCI_MAX_PATH];
    char evidence_path[NX_MMCI_MAX_PATH];
    char report_path[NX_MMCI_MAX_PATH];
    char message[NX_MMCI_MAX_MESSAGE];
} NxMmciAnalysisResult;

typedef struct NxMmciQueryResult {
    NxMmciStatus status;
    unsigned int matched_chunks;
    unsigned int confidence;
    char answer[NX_MMCI_MAX_ANSWER];
    char evidence_path[NX_MMCI_MAX_PATH];
    char message[NX_MMCI_MAX_MESSAGE];
} NxMmciQueryResult;

NxMmciStatus NxMmci_Analyze(const char* root,
                            const char* knowledge_path,
                            const char* domain,
                            NxMmciAnalysisResult* result);
NxMmciStatus NxMmci_Query(const char* root,
                          const char* analysis_id,
                          const char* question,
                          NxMmciQueryResult* result);
const char* NxMmci_StatusToString(NxMmciStatus status);

#ifdef __cplusplus
}
#endif

#endif
