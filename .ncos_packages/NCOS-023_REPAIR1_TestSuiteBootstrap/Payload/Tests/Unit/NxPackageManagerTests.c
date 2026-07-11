#include "Nexiora/NCOS/NxPackageManager.h"
#include "Nexiora/NCOS/NxPackageApply.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

_Static_assert(sizeof(NxPackageApplyResult) > 0U, "NxPackageApplyResult must be defined exactly once");

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
        NxPackageVerifyResult verify_result;
        memset(&verify_result, 0, sizeof(verify_result));
        expect(NxPackageManager_VerifyPackage(pkg, &verify_result) == 1, "verify should accept complete package");
        expect(verify_result.files_declared == 1, "verify should count declared files");
        expect(verify_result.payload_files_missing == 0, "verify should not report missing payload files");
        expect(strcmp(verify_result.package_id, "ncos-test-package") == 0, "verify should expose normalized package id");
    }

    {
        NxPackageVerifyResult deps;
        const char* dep_pkg = "Build/ncos_package_dependency_pkg";
        const char* dep_manifest = "Build/ncos_package_dependency_pkg/manifest.npkg";
        const char* dep_registry_dir = "Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/Packages/ncos-required-package";
        const char* dep_registry = "Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/Packages/ncos-required-package/registry.txt";

        make_dir(dep_pkg);
        write_text(dep_manifest,
                   "id=NCOS Dependent Package\n"
                   "version=1.0.0\n"
                   "requires=NCOS Required Package\n");
        remove_if_exists(dep_registry);
        memset(&deps, 0, sizeof(deps));
        expect(NxPackageManager_VerifyDependencies(root, dep_pkg, &deps) == 0,
               "missing dependency should block package");
        expect(deps.dependencies_declared == 1, "dependency should be declared");
        expect(deps.dependencies_missing == 1, "dependency should be reported missing");

        make_dir("Build/ncos_package_manager_test_root_isolated/Knowledge");
        make_dir("Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS");
        make_dir("Build/ncos_package_manager_test_root_isolated/Knowledge/NCOS/Packages");
        make_dir(dep_registry_dir);
        write_text(dep_registry, "id=ncos-required-package\nstatus=installed\n");
        memset(&deps, 0, sizeof(deps));
        expect(NxPackageManager_VerifyDependencies(root, dep_pkg, &deps) == 1,
               "installed dependency should satisfy package");
        expect(deps.dependencies_satisfied == 1, "dependency should be reported satisfied");
        expect(deps.dependencies_missing == 0, "no dependency should remain missing");
    }


    {
        NxPackageInstallResult tx;
        const char* txpkg = "Build/ncos_package_tx_pkg";
        const char* txmanifest = "Build/ncos_package_tx_pkg/manifest.npkg";
        const char* txpayload = "Build/ncos_package_tx_pkg/Payload/value.txt";
        const char* target = "Build/ncos_package_manager_test_root_isolated/transaction/value.txt";
        make_dir(txpkg); make_dir("Build/ncos_package_tx_pkg/Payload");
        make_dir("Build/ncos_package_manager_test_root_isolated/transaction");
        write_text(target,"original\n");
        write_text(txpayload,"updated\n");
        write_text(txmanifest,"id=NCOS Transaction Test\nversion=1.0.0\nfile=Payload/value.txt=>transaction/value.txt\n");
        memset(&tx,0,sizeof(tx));
        expect(NxPackageManager_Install(root,txpkg,&tx)==1,"transactional install should succeed");
        expect(tx.transaction_committed==1,"transaction should commit");
        expect(file_exists(tx.transaction_path),"transaction journal should exist");
        expect(tx.transaction_id[0] != '\0', "install should assign transaction id");
        {
            NxPackageHistoryResult history;
            memset(&history, 0, sizeof(history));
            expect(NxPackageManager_History(root, "NCOS Transaction Test", &history) == 1,
                   "history should expose committed transaction");
            expect(history.entries >= 1, "history should contain at least one entry");
            expect(file_exists(history.history_path), "history index should exist");
        }

        /* Regression: a second idempotent install must preserve the rollback journal. */
        memset(&tx,0,sizeof(tx));
        expect(NxPackageManager_Install(root,txpkg,&tx)==1,"second transactional install should succeed");
        expect(tx.files_skipped==1,"second transactional install should be a no-op");
        expect(file_exists(tx.transaction_path),"no-op install should preserve transaction journal");

        {
            char transaction_id[32];
            NxPackageHistoryResult history;
            memset(&history, 0, sizeof(history));
            expect(NxPackageManager_History(root, "NCOS Transaction Test", &history) == 1,
                   "history should survive no-op install");
            expect(history.entries >= 1, "no-op install should not erase history");
            snprintf(transaction_id, sizeof(transaction_id), "%s", "000001");
            memset(&tx,0,sizeof(tx));
            expect(NxPackageManager_RollbackTransaction(root,"NCOS Transaction Test",transaction_id,&tx)==1,
                   "historical rollback should succeed");
            expect(tx.files_rolled_back>=1,"historical rollback should restore at least one file");
        }
    }


    {
        NxPackageDiscoveryResult discovery;
        NxPackageInstallResult discovered_install;
        const char* discovery_root = "Build/ncos_package_discovery_root";
        const char* package_root = "Build/ncos_package_discovery_root/.ncos_packages";
        const char* pending = "Build/ncos_package_discovery_root/.ncos_packages/NCOS-PENDING";
        const char* pending_payload_dir = "Build/ncos_package_discovery_root/.ncos_packages/NCOS-PENDING/Payload";
        const char* pending_manifest = "Build/ncos_package_discovery_root/.ncos_packages/NCOS-PENDING/manifest.npkg";
        const char* pending_payload = "Build/ncos_package_discovery_root/.ncos_packages/NCOS-PENDING/Payload/pending.txt";

        make_dir(discovery_root);
        make_dir(package_root);
        make_dir(pending);
        make_dir(pending_payload_dir);
        write_text(pending_manifest,
                   "id=NCOS Pending Discovery Test\n"
                   "version=1.0.0\n"
                   "file=Payload/pending.txt=>Knowledge/NCOS/pending.txt\n");
        write_text(pending_payload, "pending package\n");
        memset(&discovery, 0, sizeof(discovery));
        expect(NxPackageManager_DiscoverPendingPackage(discovery_root, &discovery) == 1,
               "automatic discovery should find a valid pending package");
        expect(discovery.found == 1, "discovery result should be marked found");
        expect(strcmp(discovery.package_id, "ncos-pending-discovery-test") == 0,
               "discovery should expose normalized package id");
        expect(strstr(discovery.package_dir, "NCOS-PENDING") != NULL,
               "discovery should expose selected package directory");

        memset(&discovered_install, 0, sizeof(discovered_install));
        expect(NxPackageManager_Install(discovery_root, pending, &discovered_install) == 1,
               "discovered package should install successfully");
        memset(&discovery, 0, sizeof(discovery));
        expect(NxPackageManager_DiscoverPendingPackage(discovery_root, &discovery) == 0,
               "already installed package must not be discovered again");
        expect(discovery.installed_candidates >= 1,
               "discovery should count already installed packages");
    }

    expect(strcmp(NxPackageManager_ApplyPhaseName(NX_PACKAGE_APPLY_DISCOVERY), "discovery") == 0,
           "apply workflow should expose discovery phase");

    return failures == 0 ? 0 : 1;
}
