#include "Nexiora/NCOS/NxPackageManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_TEST_MKDIR(path) _mkdir(path)
#else
#define NX_TEST_MKDIR(path) mkdir((path), 0777)
#endif

static int failures = 0;

static void expect(int condition, const char* message)
{
    if (!condition) {
        printf("FAIL: %s\n", message);
        failures++;
    }
}

static void make_dir(const char* path)
{
    (void)NX_TEST_MKDIR(path);
}

static void write_text(const char* path, const char* text)
{
    FILE* f = fopen(path, "wb");
    if (f != NULL) {
        fputs(text, f);
        fclose(f);
    }
}

static int file_exists(const char* path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

int main(void)
{
    NxPackageInstallResult first;
    NxPackageInstallResult second;
    NxPackageInstallResult status;
    const char* root = "Build/ncos_package_manager_test_root";
    const char* pkg = "Build/ncos_package_manager_test_pkg";

    make_dir("Build");
    make_dir(root);
    make_dir(pkg);
    make_dir("Build/ncos_package_manager_test_pkg/Payload");

    write_text("Build/ncos_package_manager_test_pkg/manifest.npkg",
               "id=NCOS Test Package\n"
               "version=1.0.0\n"
               "file=Payload/sample.txt=>Knowledge/NCOS/PackageManagerTests/sample.txt\n");
    write_text("Build/ncos_package_manager_test_pkg/Payload/sample.txt", "sample payload\n");

    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    memset(&status, 0, sizeof(status));

    expect(NxPackageManager_Install(root, pkg, &first) == 1, "first install should succeed");
    expect(strcmp(first.package_id, "ncos-test-package") == 0, "package id should be normalized");
    expect(first.files_declared == 1, "one file should be declared");
    expect(first.files_installed == 1, "first install should copy file");
    expect(file_exists("Build/ncos_package_manager_test_root/Knowledge/NCOS/PackageManagerTests/sample.txt"),
           "installed file should exist");
    expect(file_exists(first.registry_path), "registry should exist");

    expect(NxPackageManager_Install(root, pkg, &second) == 1, "second install should succeed");
    expect(second.files_declared == 1, "second install should read manifest");
    expect(second.files_skipped == 1, "second install should skip identical file");
    expect(second.files_installed == 0, "second install should not recopy identical file");

    expect(NxPackageManager_Status(root, "NCOS Test Package", &status) == 1, "status should find installed package");
    expect(strcmp(status.package_id, "ncos-test-package") == 0, "status should normalize id");

    return failures == 0 ? 0 : 1;
}
