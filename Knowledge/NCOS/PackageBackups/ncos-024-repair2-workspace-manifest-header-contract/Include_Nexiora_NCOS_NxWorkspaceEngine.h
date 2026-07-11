#ifndef NEXIORA_NCOS_NX_WORKSPACE_ENGINE_H
#define NEXIORA_NCOS_NX_WORKSPACE_ENGINE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NxWorkspaceState {
    NX_WORKSPACE_STATE_UNKNOWN = 0,
    NX_WORKSPACE_STATE_READY = 1,
    NX_WORKSPACE_STATE_CLOSED = 2
} NxWorkspaceState;

typedef struct NxWorkspaceInfo {
    char id[128];
    char path[1024];
    char source_path[1024];
    char manifest_path[1024];
    char log_path[1024];
    size_t files_requested;
    size_t files_copied;
    size_t files_skipped;
    NxWorkspaceState state;
} NxWorkspaceInfo;

int NxWorkspace_Create(
    const char* repository_root,
    const char* workspace_id,
    const char* const* relative_files,
    size_t file_count,
    NxWorkspaceInfo* out_info);

int NxWorkspace_Status(
    const char* repository_root,
    const char* workspace_id,
    NxWorkspaceInfo* out_info);

int NxWorkspace_Close(
    const char* repository_root,
    const char* workspace_id,
    NxWorkspaceInfo* out_info);

int NxWorkspace_Clean(
    const char* repository_root,
    const char* workspace_id);

const char* NxWorkspace_StateName(NxWorkspaceState state);

#ifdef __cplusplus
}
#endif

#endif
