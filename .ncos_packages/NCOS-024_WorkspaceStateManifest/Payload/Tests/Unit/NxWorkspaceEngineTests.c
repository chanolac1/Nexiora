#include "Nexiora/NCOS/NxWorkspaceEngine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#endif

static int failures = 0;

static void nx_expect(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static int nx_write_file(const char* path, const char* content)
{
    FILE* file = fopen(path, "wb");
    if (file == NULL) {
        return 0;
    }
    fputs(content, file);
    return fclose(file) == 0;
}

static int nx_file_contains(const char* path, const char* expected)
{
    FILE* file;
    char buffer[512];
    size_t amount;
    file = fopen(path, "rb");
    if (file == NULL) {
        return 0;
    }
    amount = fread(buffer, 1U, sizeof(buffer) - 1U, file);
    fclose(file);
    buffer[amount] = '\0';
    return strstr(buffer, expected) != NULL;
}

int main(void)
{
    const char* root = "Build/NxWorkspaceEngineTestsSandbox";
    const char* files[] = {"alpha.txt", "missing.txt", "../unsafe.txt"};
    NxWorkspaceInfo created;
    NxWorkspaceInfo status;
    char copied[1024];

    (void)NxWorkspace_Clean(root, "Test Workspace");
    (void)NX_MKDIR("Build");
    (void)NX_MKDIR(root);
    nx_expect(nx_write_file("Build/NxWorkspaceEngineTestsSandbox/alpha.txt", "workspace evidence\n"), "fixture should be created");

    nx_expect(NxWorkspace_Create(root, "Test Workspace", files, 3U, &created), "create should succeed");
    nx_expect(strcmp(created.id, "test-workspace") == 0, "id should be normalized");
    nx_expect(created.state == NX_WORKSPACE_STATE_READY, "workspace should be ready");
    nx_expect(created.files_requested == 3U, "requested count should be preserved");
    nx_expect(created.files_copied == 1U, "one valid source should be copied");
    nx_expect(created.files_skipped == 2U, "missing and unsafe sources should be skipped");

    nx_expect(strlen(created.source_path) + strlen("/alpha.txt") < sizeof(copied), "copied path should fit");
    copied[0] = '\0';
    if (strlen(created.source_path) + strlen("/alpha.txt") < sizeof(copied)) {
        memcpy(copied, created.source_path, strlen(created.source_path) + 1U);
        memcpy(copied + strlen(copied), "/alpha.txt", strlen("/alpha.txt") + 1U);
    }
    nx_expect(nx_file_contains(copied, "workspace evidence"), "copied source should preserve content");
    nx_expect(nx_file_contains(created.manifest_path, "state=READY"), "manifest should record ready state");
    nx_expect(nx_file_contains(created.log_path, "COPY|alpha.txt"), "action log should record copied file");

    nx_expect(!NxWorkspace_Create(root, "Test Workspace", files, 3U, NULL), "duplicate workspace should be rejected");
    nx_expect(NxWorkspace_Status(root, "Test Workspace", &status), "status should succeed");
    nx_expect(status.files_copied == 1U, "status should restore copied count");
    nx_expect(NxWorkspace_Close(root, "Test Workspace", &status), "close should succeed");
    nx_expect(status.state == NX_WORKSPACE_STATE_CLOSED, "close should persist closed state");
    nx_expect(NxWorkspace_Status(root, "Test Workspace", &status), "closed status should load");
    nx_expect(status.state == NX_WORKSPACE_STATE_CLOSED, "status should report closed");
    nx_expect(NxWorkspace_Clean(root, "Test Workspace"), "clean should succeed");
    nx_expect(!NxWorkspace_Status(root, "Test Workspace", NULL), "cleaned workspace should not exist");

    remove("Build/NxWorkspaceEngineTestsSandbox/alpha.txt");

    {
        const char* manifest_path = "Build/NxWorkspaceEngineTestsSandbox/workspace.nxws";
        NxWorkspaceManifestResult manifest_result;
        nx_expect(nx_write_file("Build/NxWorkspaceEngineTestsSandbox/CMakeLists.txt",
                                "add_executable(alpha alpha.c)\nadd_test(NAME Alpha COMMAND alpha)\n"),
                  "manifest cmake fixture should be created");
        nx_expect(nx_write_file("Build/NxWorkspaceEngineTestsSandbox/CMakePresets.json", "{}\n"),
                  "manifest presets fixture should be created");
        nx_expect(NxWorkspaceManifest_Snapshot(root, manifest_path, &manifest_result),
                  "workspace snapshot should succeed");
        nx_expect(manifest_result.target_count == 1U, "snapshot should discover target count");
        nx_expect(manifest_result.test_count == 1U, "snapshot should discover test count");
        nx_expect(NxWorkspaceManifest_Diff(root, manifest_path, &manifest_result),
                  "fresh snapshot should match workspace");
        nx_expect(nx_write_file("Build/NxWorkspaceEngineTestsSandbox/CMakeLists.txt",
                                "add_executable(alpha alpha.c)\nadd_executable(beta beta.c)\nadd_test(NAME Alpha COMMAND alpha)\n"),
                  "changed manifest cmake fixture should be created");
        nx_expect(!NxWorkspaceManifest_Diff(root, manifest_path, &manifest_result),
                  "changed workspace should differ from snapshot");
        remove(manifest_path);
        remove("Build/NxWorkspaceEngineTestsSandbox/CMakeLists.txt");
        remove("Build/NxWorkspaceEngineTestsSandbox/CMakePresets.json");
    }

    return failures == 0 ? 0 : 1;
}
