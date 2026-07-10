#ifndef NEXIORA_MULTIMODAL_NX_MULTIMODAL_INGESTION_H
#define NEXIORA_MULTIMODAL_NX_MULTIMODAL_INGESTION_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NX_MM_MAX_PATH 512
#define NX_MM_MAX_MESSAGE 256
#define NX_MM_MAX_TITLE 128

typedef enum NxMultimodalStatus {
    NX_MM_OK = 0,
    NX_MM_PARTIAL = 1,
    NX_MM_INVALID_ARGUMENT = 2,
    NX_MM_NOT_FOUND = 3,
    NX_MM_UNSUPPORTED = 4,
    NX_MM_IO_ERROR = 5,
    NX_MM_OUTPUT_TOO_SMALL = 6,
    NX_MM_TRANSCRIPT_REQUIRED = 7,
    NX_MM_FORMAT_ERROR = 8
} NxMultimodalStatus;

typedef enum NxMediaType {
    NX_MEDIA_UNKNOWN = 0,
    NX_MEDIA_TEXT = 1,
    NX_MEDIA_MARKDOWN = 2,
    NX_MEDIA_PDF = 3,
    NX_MEDIA_WAV = 4,
    NX_MEDIA_AUDIO = 5,
    NX_MEDIA_VIDEO = 6,
    NX_MEDIA_IMAGE = 7
} NxMediaType;

typedef struct NxMediaProbe {
    NxMediaType type;
    unsigned long long file_size;
    unsigned int sample_rate;
    unsigned int channels;
    unsigned int duration_ms;
    int transcript_available;
    char source_path[NX_MM_MAX_PATH];
    char transcript_path[NX_MM_MAX_PATH];
} NxMediaProbe;

typedef struct NxIngestionResult {
    NxMultimodalStatus status;
    NxMediaType type;
    size_t extracted_bytes;
    char knowledge_path[NX_MM_MAX_PATH];
    char message[NX_MM_MAX_MESSAGE];
} NxIngestionResult;

const char* NxMultimodal_StatusToString(NxMultimodalStatus status);
const char* NxMultimodal_MediaTypeToString(NxMediaType type);
NxMultimodalStatus NxMultimodal_Probe(const char* source_path, NxMediaProbe* probe_out);
NxMultimodalStatus NxMultimodal_Ingest(const char* root_path, const char* source_path,
    const char* title, NxIngestionResult* result_out);

#ifdef __cplusplus
}
#endif
#endif
