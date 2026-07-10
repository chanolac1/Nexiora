#include "Nexiora/CognitivePipeline/NxWebCognitivePipeline.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_RMDIR(path) _rmdir(path)
#define NX_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define NX_MKDIR(path) mkdir((path), 0777)
#define NX_RMDIR(path) rmdir(path)
#define NX_GETPID() getpid()
#endif

static int require_true(int condition, const char* message)
{
    if (!condition) {
        fprintf(stderr, "NxWebCognitivePipelineTests: FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static int join_path(char* output, size_t output_size, const char* left, const char* right)
{
    const int written = snprintf(output, output_size, "%s/%s", left, right);
    return written > 0 && (size_t)written < output_size;
}

static int write_fixture(const char* path)
{
    FILE* file = fopen(path, "wb");
    if (file == NULL) { return 0; }
    if (fputs("WEBVTT\n\n00:00:00.000 --> 00:00:03.000\nNexiora aprende de evidencia verificable.\n\n"
              "00:00:03.000 --> 00:00:07.000\nLa memoria semantica conserva conceptos y relaciones.\n\n"
              "00:00:07.000 --> 00:00:10.000\nLa evidencia permite responder sin inventar.\n", file) == EOF) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

int main(void)
{
    NxWcpPlan plan;
    NxWcpStatus status;
    FILE* knowledge;
    char root[96];
    char fixture_path[NX_WCP_MAX_PATH];
    char knowledge_path[NX_WCP_MAX_PATH];
    char variant_path[NX_WCP_MAX_PATH];
    char text[4096];
    char analysis_id[192];
    char message[NX_WCP_MAX_MESSAGE];
    size_t bytes;
    const int root_written = snprintf(root, sizeof(root), "nx_wcp_test_%ld", (long)NX_GETPID());

    if (!require_true(root_written > 0 && (size_t)root_written < sizeof(root), "test root path overflow")) { return 1; }
    if (NX_MKDIR(root) != 0) { return require_true(0, "test root could not be created") ? 0 : 1; }

    status = NxWcp_BuildPlan(root, "https://www.youtube.com/watch?v=abc_DEF-123", "es", &plan);
    if (!require_true(status == NX_WCP_OK, "valid YouTube URL was not planned")) { return 1; }
    if (!require_true(strcmp(plan.source_id, "abc_DEF-123") == 0, "YouTube id was not extracted")) { return 1; }
    if (!require_true(strstr(plan.download_command, "yt-dlp") != NULL, "yt-dlp plan was not created")) { return 1; }

    if (!require_true(join_path(variant_path, sizeof(variant_path), plan.workspace, "abc_DEF-123.es.es.vtt"), "variant transcript path overflow")) { return 1; }
    if (!write_fixture(variant_path)) { return require_true(0, "variant transcript fixture could not be written") ? 0 : 1; }
    status = NxWcp_FindTranscript(&plan, text, sizeof(text));
    if (!require_true(status == NX_WCP_OK, "variant yt-dlp transcript name was not discovered")) { return 1; }

    if (!require_true(join_path(fixture_path, sizeof(fixture_path), root, "sample.es.vtt"), "fixture path overflow")) { return 1; }
    if (!require_true(join_path(knowledge_path, sizeof(knowledge_path), root, "sample.nxknowledge"), "knowledge path overflow")) { return 1; }
    if (!write_fixture(fixture_path)) { return require_true(0, "fixture could not be written") ? 0 : 1; }

    status = NxWcp_ConvertWebVtt(fixture_path, "https://youtu.be/abc_DEF-123", "Demostracion", knowledge_path);
    if (!require_true(status == NX_WCP_OK, "WebVTT conversion failed")) { return 1; }
    knowledge = fopen(knowledge_path, "rb");
    if (!require_true(knowledge != NULL, "knowledge output was not created")) { return 1; }
    bytes = fread(text, 1U, sizeof(text) - 1U, knowledge);
    text[bytes] = '\0';
    fclose(knowledge);
    if (!require_true(strstr(text, "nxknowledge/1") != NULL, "knowledge header missing")) { return 1; }
    if (!require_true(strstr(text, "content_begin\n") != NULL, "content_begin marker missing")) { return 1; }
    if (!require_true(strstr(text, "\ncontent_end") != NULL, "content_end marker missing")) { return 1; }

    status = NxWcp_IntegrateKnowledge(root, knowledge_path, "web_learning",
                                      analysis_id, sizeof(analysis_id), message, sizeof(message));
    if (!require_true(status == NX_WCP_OK, message[0] == '\0' ? "direct cognitive integration failed" : message)) { return 1; }
    if (!require_true(analysis_id[0] != '\0', "analysis id was not returned")) { return 1; }
    if (!require_true(strstr(message, "integrated directly") != NULL, "direct integration diagnostic missing")) { return 1; }

    (void)remove(variant_path);
    (void)remove(fixture_path);
    (void)remove(knowledge_path);
    (void)NX_RMDIR(plan.workspace);
    
    CHECK(strstr(plan.download_command, "youtube:player_client=default") != NULL, "subtitle-only client policy missing");
    CHECK(strstr(plan.download_command, "-f best") != NULL, "single-format policy missing");
puts("NxWebCognitivePipelineTests: PASS");
    return 0;
}
