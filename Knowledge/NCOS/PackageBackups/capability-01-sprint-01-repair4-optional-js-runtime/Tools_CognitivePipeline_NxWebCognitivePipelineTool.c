#include "Nexiora/CognitivePipeline/NxWebCognitivePipeline.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    puts("Usage:");
    puts("  nexiora_web_cognitive plan <root> <youtube_url> <language>");
    puts("  nexiora_web_cognitive convert <transcript.vtt> <source_url> <title> <output.nxknowledge>");
    puts("  nexiora_web_cognitive learn <root> <youtube_url> <language> <domain>");
}

static const char* runtime_status(const NxWcpPlan* plan)
{
    if (plan == NULL) { return "INVALID"; }
    return strstr(plan->download_command, "--js-runtimes") != NULL &&
           strstr(plan->download_command, "deno.exe") != NULL ? "CONFIGURED" : "MISSING";
}

int main(int argc, char** argv)
{
    if (argc >= 2 && strcmp(argv[1], "plan") == 0) {
        NxWcpPlan plan;
        NxWcpStatus status;
        if (argc != 5) { usage(); return 2; }
        status = NxWcp_BuildPlan(argv[2], argv[3], argv[4], &plan);
        printf("status=%s\nsource_id=%s\nworkspace=%s\ntranscript=%s\nknowledge=%s\nmanaged_js_runtime=%s\ncommand=%s\nmessage=%s\n",
               NxWcp_StatusToString(status), plan.source_id, plan.workspace,
               plan.transcript_path, plan.knowledge_path, runtime_status(&plan),
               plan.download_command, plan.message);
        return status == NX_WCP_OK ? 0 : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "convert") == 0) {
        NxWcpStatus status;
        if (argc != 6) { usage(); return 2; }
        status = NxWcp_ConvertWebVtt(argv[2], argv[3], argv[4], argv[5]);
        printf("status=%s\noutput=%s\n", NxWcp_StatusToString(status), argv[5]);
        return status == NX_WCP_OK ? 0 : 1;
    }
    if (argc >= 2 && strcmp(argv[1], "learn") == 0) {
        NxWcpResult result;
        NxWcpStatus status;
        if (argc != 6) { usage(); return 2; }
        status = NxWcp_Execute(argv[2], argv[3], argv[4], argv[5], &result);
        printf("status=%s\nsource_id=%s\ntranscript=%s\nknowledge=%s\nanalysis_id=%s\nmessage=%s\n",
               NxWcp_StatusToString(status), result.source_id, result.transcript_path,
               result.knowledge_path, result.analysis_id, result.message);
        return status == NX_WCP_OK ? 0 : 1;
    }
    usage();
    return 2;
}
