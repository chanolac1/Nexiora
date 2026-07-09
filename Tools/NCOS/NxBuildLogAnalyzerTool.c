#include "Nexiora/NCOS/NxBuildLogAnalyzer.h"
#include <stdio.h>
#include <string.h>

static void usage(void)
{
    printf("NEXIORA - Build Log Analyzer\n\n");
    printf("Uso:\n");
    printf("  nexiora_analyze_log analyze <run_id> <log_path>\n");
}

int main(int argc, char** argv)
{
    NxBuildLogAnalysis a;
    int i;
    if (argc < 4 || strcmp(argv[1], "analyze") != 0) {
        usage();
        return 1;
    }

    if (!NxBuildLogAnalyzer_AnalyzeFile(".", argv[2], argv[3], &a)) {
        printf("No fue posible analizar el log.\n");
        printf("Resumen: %s\n", a.summary);
        return 2;
    }

    printf("================================================\n");
    printf(" NEXIORA - Build Log Analyzer\n");
    printf("================================================\n\n");
    printf("Run ID       : %s\n", argv[2]);
    printf("Log          : %s\n", argv[3]);
    printf("Estado       : %s\n", a.success ? "OK" : "FAILED");
    printf("Errores      : %d\n", a.error_count);
    printf("Warnings     : %d\n", a.warning_count);
    printf("Fallas tests : %d\n", a.failure_count);
    printf("Hallazgos    : %d\n", a.finding_count);
    printf("Reporte      : %s\n\n", a.report_path);
    printf("Resumen      : %s\n", a.summary);

    if (a.finding_count > 0) {
        printf("\nHallazgos principales:\n");
        for (i = 0; i < a.finding_count && i < 8; ++i) {
            printf("  %d. [%s] %s\n", i + 1, a.findings[i].type, a.findings[i].message);
        }
    }

    return a.success ? 0 : 8;
}
