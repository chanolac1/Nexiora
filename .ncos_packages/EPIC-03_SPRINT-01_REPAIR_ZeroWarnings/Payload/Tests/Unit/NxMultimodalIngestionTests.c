#include "Nexiora/Multimodal/NxMultimodalIngestion.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define nx_mkdir(path) _mkdir(path)
#else
#include <sys/stat.h>
#define nx_mkdir(path) mkdir((path), 0777)
#endif

static void fail_test(const char* message)
{
    (void)fprintf(stderr, "NxMultimodalIngestionTests: FAIL: %s\n", message);
    exit(EXIT_FAILURE);
}

static void require_true(int condition, const char* message)
{
    if (condition == 0) {
        fail_test(message);
    }
}

static void write_text(const char* path, const char* text)
{
    FILE* file;
    size_t text_length;
    size_t written;

    require_true(path != NULL, "write_text path is null");
    require_true(text != NULL, "write_text text is null");

    file = fopen(path, "wb");
    require_true(file != NULL, "unable to create test file");

    text_length = strlen(text);
    written = fwrite(text, 1U, text_length, file);
    if (written != text_length) {
        (void)fclose(file);
        fail_test("unable to write complete test file");
    }

    require_true(fclose(file) == 0, "unable to close test file");
}

static void remove_if_present(const char* path)
{
    if (remove(path) != 0) {
        FILE* file = fopen(path, "rb");
        if (file != NULL) {
            (void)fclose(file);
            fail_test("unable to remove generated test artifact");
        }
    }
}

int main(void)
{
    NxMediaProbe probe;
    NxIngestionResult result;
    NxMultimodalStatus status;

    (void)nx_mkdir("nx_mm_test");

    write_text("nx_mm_test/a.txt", "Nexiora aprende contenido verificable.\n");
    status = NxMultimodal_Probe("nx_mm_test/a.txt", &probe);
    require_true(status == NX_MM_OK, "text probe failed");
    require_true(probe.type == NX_MEDIA_TEXT, "text probe returned wrong media type");

    status = NxMultimodal_Ingest(
        "nx_mm_test",
        "nx_mm_test/a.txt",
        "Prueba Texto",
        &result);
    require_true(status == NX_MM_OK, "text ingestion failed");
    require_true(result.extracted_bytes > 0U, "text ingestion extracted no bytes");
    require_true(
        strstr(result.knowledge_path, "prueba_texto.nxknowledge") != NULL,
        "text knowledge path is incorrect");

    write_text("nx_mm_test/video.mp4", "fake");
    status = NxMultimodal_Ingest(
        "nx_mm_test",
        "nx_mm_test/video.mp4",
        "Video",
        &result);
    require_true(
        status == NX_MM_TRANSCRIPT_REQUIRED,
        "video without transcript was not rejected");

    write_text("nx_mm_test/video.txt", "Transcripcion verificable.\n");
    status = NxMultimodal_Ingest(
        "nx_mm_test",
        "nx_mm_test/video.mp4",
        "Video",
        &result);
    require_true(status == NX_MM_OK, "video ingestion with transcript failed");

    remove_if_present("nx_mm_test/a.txt");
    remove_if_present("nx_mm_test/video.mp4");
    remove_if_present("nx_mm_test/video.txt");
    remove_if_present("nx_mm_test/Knowledge/Multimodal/prueba_texto.nxknowledge");
    remove_if_present("nx_mm_test/Knowledge/Multimodal/video.nxknowledge");

    (void)puts("NxMultimodalIngestionTests: PASS");
    return EXIT_SUCCESS;
}
