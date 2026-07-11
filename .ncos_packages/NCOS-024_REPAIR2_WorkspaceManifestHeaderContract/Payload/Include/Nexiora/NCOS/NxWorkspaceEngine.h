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

/* NCOS-024 Workspace State Manifest */
typedef enum NxWorkspaceManifestStatus {
    NX_WORKSPACE_MANIFEST_OK = 0,
    NX_WORKSPACE_MANIFEST_INVALID_ARGUMENT = 1,
    NX_WORKSPACE_MANIFEST_IO_ERROR = 2,
    NX_WORKSPACE_MANIFEST_COMMAND_ERROR = 3,
    NX_WORKSPACE_MANIFEST_OUT_OF_SYNC = 4
} NxWorkspaceManifestStatus;

typedef struct NxWorkspaceManifestResult {
    NxWorkspaceManifestStatus status;
    char manifest_path[1024];
    char git_branch[128];
    char git_commit[128];
    int git_dirty;
    size_t target_count;
    size_t test_count;
    size_t executable_count;
    size_t installed_package_count;
    size_t pending_package_count;
    size_t blocked_package_count;
    size_t documentation_checked;
    size_t documentation_missing;
    unsigned long long source_inventory_hash;
    unsigned long long cmake_inventory_hash;
    unsigned long long package_inventory_hash;
    size_t differences;
    char message[512];
} NxWorkspaceManifestResult;

int NxWorkspaceManifest_Snapshot(const char* repository_root,
                                 const char* output_path,
                                 NxWorkspaceManifestResult* out_result);
int NxWorkspaceManifest_Validate(const char* repository_root,
                                 const char* manifest_path,
                                 NxWorkspaceManifestResult* out_result);
int NxWorkspaceManifest_Diff(const char* repository_root,
                             const char* manifest_path,
                             NxWorkspaceManifestResult* out_result);
const char* NxWorkspaceManifest_StatusName(NxWorkspaceManifestStatus status);

#ifdef __cplusplus
}
#endif

#endif
