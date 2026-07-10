#ifndef NEXIORA_COGNITIVE_PIPELINE_NX_WEB_COGNITIVE_PIPELINE_H
#define NEXIORA_COGNITIVE_PIPELINE_NX_WEB_COGNITIVE_PIPELINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_WCP_MAX_PATH 512U
#define NX_WCP_MAX_URL 1024U
#define NX_WCP_MAX_COMMAND 4096U
#define NX_WCP_MAX_MESSAGE 256U

typedef enum NxWcpStatus {
    NX_WCP_OK = 0,
    NX_WCP_INVALID_ARGUMENT = 1,
    NX_WCP_UNSUPPORTED_URL = 2,
    NX_WCP_TOOL_NOT_FOUND = 3,
    NX_WCP_SOURCE_NOT_FOUND = 4,
    NX_WCP_IO_ERROR = 5,
    NX_WCP_FORMAT_ERROR = 6,
    NX_WCP_COMMAND_FAILED = 7,
    NX_WCP_OUTPUT_TOO_SMALL = 8
} NxWcpStatus;

typedef struct NxWcpPlan {
    NxWcpStatus status;
    char url[NX_WCP_MAX_URL];
    char source_id[128];
    char language[32];
    char workspace[NX_WCP_MAX_PATH];
    char transcript_path[NX_WCP_MAX_PATH];
    char knowledge_path[NX_WCP_MAX_PATH];
    char download_command[NX_WCP_MAX_COMMAND];
    char message[NX_WCP_MAX_MESSAGE];
} NxWcpPlan;

typedef struct NxWcpResult {
    NxWcpStatus status;
    char source_id[128];
    char transcript_path[NX_WCP_MAX_PATH];
    char knowledge_path[NX_WCP_MAX_PATH];
    char analysis_id[192];
    char message[NX_WCP_MAX_MESSAGE];
} NxWcpResult;

NxWcpStatus NxWcp_BuildPlan(const char* root,
                            const char* url,
                            const char* language,
                            NxWcpPlan* plan);
NxWcpStatus NxWcp_ConvertWebVtt(const char* transcript_path,
                                const char* source_url,
                                const char* title,
                                const char* knowledge_path);
NxWcpStatus NxWcp_FindTranscript(const NxWcpPlan* plan,
                                      char* found,
                                      size_t found_size);
NxWcpStatus NxWcp_IntegrateKnowledge(const char* root,
                                      const char* knowledge_path,
                                      const char* domain,
                                      char* analysis_id,
                                      size_t analysis_id_size,
                                      char* message,
                                      size_t message_size);
NxWcpStatus NxWcp_Execute(const char* root,
                          const char* url,
                          const char* language,
                          const char* domain,
                          NxWcpResult* result);
int NxWcp_IsYouTubeUrl(const char* url);
const char* NxWcp_StatusToString(NxWcpStatus status);

#ifdef __cplusplus
}
#endif

#endif
