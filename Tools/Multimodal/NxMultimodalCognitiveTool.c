#include "Nexiora/Multimodal/NxMultimodalCognitiveIntegration.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    puts("Usage:");
    puts("  nexiora_multimodal_cognitive analyze <root> <knowledge.nxknowledge> <domain>");
    puts("  nexiora_multimodal_cognitive query <root> <analysis_id> <question>");
}

int main(int argc, char** argv)
{
    if (argc >= 2 && strcmp(argv[1], "analyze") == 0) {
        NxMmciAnalysisResult result;
        NxMmciStatus status;
        if (argc != 5) { usage(); return 2; }
        status = NxMmci_Analyze(argv[2], argv[3], argv[4], &result);
        printf("status=%s\nchunks=%u\nconcepts=%u\nrelations=%u\nconfidence=%u\nanalysis_dir=%s\nevidence=%s\nreport=%s\nmessage=%s\n",
               NxMmci_StatusToString(status), result.chunks_written, result.concepts_written,
               result.relations_written, result.confidence, result.analysis_dir,
               result.evidence_path, result.report_path, result.message);
        return status == NX_MMCI_OK ? 0 : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "query") == 0) {
        NxMmciQueryResult result;
        NxMmciStatus status;
        if (argc != 5) { usage(); return 2; }
        status = NxMmci_Query(argv[2], argv[3], argv[4], &result);
        printf("status=%s\nmatched_chunks=%u\nconfidence=%u\nevidence=%s\nmessage=%s\nanswer=%s\n",
               NxMmci_StatusToString(status), result.matched_chunks, result.confidence,
               result.evidence_path, result.message, result.answer);
        return status == NX_MMCI_OK ? 0 : 1;
    }
    usage();
    return 2;
}
