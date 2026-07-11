#include "Nexiora/NCOS/NxPackageManager.h"
#include "Nexiora/NCOS/NxPackageCertification.h"
#include <time.h>

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

static void remove_if_exists(const char* path)
{
    (void)remove(path);
}

int main(void)
{
    NxPackageInstallResult first;
    NxPackageInstallResult second;
    NxPackageInstallResult status;

    const char* root = "Build/ncos_package_manager_test_root_isolated";
    const char* pkg = "Build/ncos_package_manager_test_pkg_isolated";
    const char* manifest = "Build/ncos_package_manager_test_pkg_isolated/manifest.npkg";
    const char* payload = "Build/ncos_package_manager_test_pkg_isolated/Payload/sample.txt";
    const char* installed = "Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/PackageManagerTests/sample.txt";
    const char* registry = "Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/Packages/ncos-test-package/registry.txt";
    const char* log = "Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/Packages/ncos-test-package/install.log";

    make_dir("Build");
    make_dir(root);
    make_dir(pkg);
    make_dir("Build/ncos_package_manager_test_pkg_isolated/Payload");

    /* Ensure the test is repeatable even after previous executions or package payload extraction. */
    remove_if_exists(installed);
    remove_if_exists(registry);
    remove_if_exists(log);

    write_text(manifest,
               "id=NCOS Test Package\n"
               "version=1.0.0\n"
               "file=Payload/sample.txt=>Knowledge/NCOS/PackageManagerTests/sample.txt\n");
    write_text(payload, "sample payload\n");

    memset(&first, 0, sizeof(first));
    memset(&second, 0, sizeof(second));
    memset(&status, 0, sizeof(status));

    expect(NxPackageManager_Install(root, pkg, &first) == 1, "first install should succeed");
    expect(strcmp(first.package_id, "ncos-test-package") == 0, "package id should be normalized");
    expect(first.files_declared == 1, "one file should be declared");
    expect(first.files_installed == 1, "first install should copy file");
    expect(file_exists(installed), "installed file should exist");
    expect(file_exists(first.registry_path), "registry should exist");

    expect(NxPackageManager_Install(root, pkg, &second) == 1, "second install should succeed");
    expect(second.files_declared == 1, "second install should read manifest");
    expect(second.files_skipped == 1, "second install should skip identical file");
    expect(second.files_installed == 0, "second install should not recopy identical file");

    expect(NxPackageManager_Status(root, "NCOS Test Package", &status) == 1, "status should find installed package");
    expect(strcmp(status.package_id, "ncos-test-package") == 0, "status should normalize id");

    {
        NxPackageApplyResult apply_result;
        NxPackageCertificationSummary certification;
        const char* cert_manifest = "Build/ncos_package_manager_test_pkg_isolated/manifest.npkg";
        const char* cert_artifact = "Build/ncos_package_manager_test_root_isolated/required.exe";
        const char* cert_log = "Build/ncos_package_manager_test_root_isolated/apply.log";
        memset(&apply_result, 0, sizeof(apply_result));
        memset(&certification, 0, sizeof(certification));
        write_text(cert_manifest,
                   "id=NCOS Test Package\n"
                   "version=1.0.0\n"
                   "artifact=required.exe\n"
                   "file=Payload/sample.txt=>Knowledge/NCOS/PackageManagerTests/sample.txt\n");
        write_text(cert_artifact, "artifact\n");
        write_text(cert_log,
                   "100% tests passed, 0 tests failed out of 2\n"
                   "[NCOS-025] phase=apply state=completed\n");
        strcpy(apply_result.package_id, "ncos-test-package");
        strcpy(apply_result.package_version, "1.0.0");
        strcpy(apply_result.apply_log_path, cert_log);
        apply_result.verify_passed = 1;
        apply_result.dependencies_passed = 1;
        apply_result.install_passed = 1;
        apply_result.configure_passed = 1;
        apply_result.build_passed = 1;
        apply_result.warning_gate_passed = 1;
        apply_result.tests_passed = 1;
        apply_result.documentation_passed = 1;
        apply_result.artifacts_passed = 1;
        apply_result.qa_repeatability_passed = 1;
        apply_result.success = 1;
        expect(NxPackageCertification_Generate(root, pkg, &apply_result,
                                               (time_t)100, (time_t)105, 2, 2,
                                               &certification) == 1,
               "certification report should be generated");
        expect(strcmp(certification.final_status, "CERTIFIED") == 0,
               "successful apply should be certified");
        expect(strcmp(certification.release_recommendation, "APPROVE_GIT") == 0,
               "certified apply should recommend Git approval");
        expect(certification.artifacts_missing == 0,
               "required artifact should be present");
        expect(file_exists(certification.nxcert_path),
               "machine-readable certification should exist");
        expect(file_exists(certification.text_path),
               "human-readable certification should exist");
    }

    return failures == 0 ? 0 : 1;
}
