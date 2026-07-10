#include "Nexiora/CognitivePipeline/NxWebCognitivePipeline.h"

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

static int require_true(int condition, const char* message)
{
    if (!condition) { fprintf(stderr, "NxWebCognitivePipelineTests: FAIL: %s\n", message); return 0; }
    return 1;
}

static int write_fixture(const char* path)
{
    FILE* file = fopen(path, "wb");
    if (file == NULL) { return 0; }
    fputs("WEBVTT\n\n00:00:00.000 --> 00:00:03.000\nNexiora aprende de evidencia verificable.\n\n"
          "00:00:03.000 --> 00:00:07.000\nLa memoria semantica conserva conceptos y relaciones.\n\n"
          "00:00:07.000 --> 00:00:10.000\nLa evidencia permite responder sin inventar.\n", file);
    return fclose(file) == 0;
}

int main(void)
{
    NxWcpPlan plan;
    NxWcpStatus status;
    FILE* knowledge;
    char text[4096];
    size_t bytes;
    (void)system("rm -rf nx_wcp_test 2>/dev/null");
    (void)system("rmdir /s /q nx_wcp_test >nul 2>&1");
    if (NX_MKDIR("nx_wcp_test") != 0) { return require_true(0, "test root could not be created") ? 0 : 1; }
    status = NxWcp_BuildPlan("nx_wcp_test", "https://www.youtube.com/watch?v=abc_DEF-123", "es", &plan);
    if (!require_true(status == NX_WCP_OK, "valid YouTube URL was not planned")) { return 1; }
    if (!require_true(strcmp(plan.source_id, "abc_DEF-123") == 0, "YouTube id was not extracted")) { return 1; }
    if (!require_true(strstr(plan.download_command, "yt-dlp") != NULL, "yt-dlp plan was not created")) { return 1; }
    status = NxWcp_BuildPlan("nx_wcp_test", "https://example.com/video", "es", &plan);
    if (!require_true(status == NX_WCP_UNSUPPORTED_URL, "unsupported URL was accepted")) { return 1; }
    if (!write_fixture("nx_wcp_test/sample.es.vtt")) { return require_true(0, "fixture could not be written") ? 0 : 1; }
    status = NxWcp_ConvertWebVtt("nx_wcp_test/sample.es.vtt", "https://youtu.be/abc_DEF-123", "Demostracion", "nx_wcp_test/sample.nxknowledge");
    if (!require_true(status == NX_WCP_OK, "WebVTT conversion failed")) { return 1; }
    knowledge = fopen("nx_wcp_test/sample.nxknowledge", "rb");
    if (!require_true(knowledge != NULL, "knowledge output was not created")) { return 1; }
    bytes = fread(text, 1U, sizeof(text) - 1U, knowledge);
    text[bytes] = '\0';
    fclose(knowledge);
    if (!require_true(strstr(text, "nxknowledge/1") != NULL, "knowledge header missing")) { return 1; }
    if (!require_true(strstr(text, "source_url=https://youtu.be/abc_DEF-123") != NULL, "source provenance missing")) { return 1; }
    if (!require_true(strstr(text, "memoria semantica") != NULL, "transcript evidence missing")) { return 1; }
    puts("NxWebCognitivePipelineTests: PASS");
    return 0;
}
