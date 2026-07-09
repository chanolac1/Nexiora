#include "Nexiora/NCOS/NxAutoFixProposalEngine.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#define NX_SEP "\\"
#else
#include <unistd.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#define NX_SEP "/"
#endif

static void nx_zero(void* p, size_t n) { memset(p, 0, n); }

static void nx_lower_copy(char* dst, size_t dst_size, const char* src)
{
    size_t i;
    if (!dst || dst_size == 0) return;
    if (!src) src = "";
    for (i = 0; i + 1 < dst_size && src[i]; ++i) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = 0;
}

static int nx_contains_ci(const char* text, const char* needle)
{
    char a[8192];
    char b[256];
    nx_lower_copy(a, sizeof(a), text);
    nx_lower_copy(b, sizeof(b), needle);
    return strstr(a, b) != NULL;
}

static void nx_safe_id(char* dst, size_t dst_size, const char* src)
{
    size_t i, j = 0;
    if (!dst || dst_size == 0) return;
    if (!src || !*src) src = "proposal";
    for (i = 0; src[i] && j + 1 < dst_size; ++i) {
        unsigned char c = (unsigned char)src[i];
        if (isalnum(c)) dst[j++] = (char)tolower(c);
        else if (c == '_' || c == '-') dst[j++] = (char)c;
        else if (j > 0 && dst[j - 1] != '_') dst[j++] = '_';
    }
    if (j == 0) dst[j++] = 'p';
    dst[j] = 0;
}

static int nx_join(char* dst, size_t dst_size, const char* a, const char* b)
{
    size_t la;
    const char* sep = NX_SEP;
    int n;
    if (!dst || dst_size == 0 || !a || !b) return 0;
    la = strlen(a);
    if (la > 0 && (a[la - 1] == '/' || a[la - 1] == '\\')) sep = "";
    n = snprintf(dst, dst_size, "%s%s%s", a, sep, b);
    return n > 0 && (size_t)n < dst_size;
}

static void nx_mkdir_if_missing(const char* path)
{
    if (!path || !*path) return;
    NX_MKDIR(path);
}

static int nx_ensure_dirs(const char* root, char* proposal_dir, size_t proposal_dir_size)
{
    char knowledge[512];
    char ncos[512];
    const char* base = (root && *root) ? root : ".";
    nx_mkdir_if_missing(base);
    if (!nx_join(knowledge, sizeof(knowledge), base, "Knowledge")) return 0;
    nx_mkdir_if_missing(knowledge);
    if (!nx_join(ncos, sizeof(ncos), knowledge, "NCOS")) return 0;
    nx_mkdir_if_missing(ncos);
    if (!nx_join(proposal_dir, proposal_dir_size, ncos, "FixProposals")) return 0;
    nx_mkdir_if_missing(proposal_dir);
    return 1;
}

static void nx_add_suggestion(NxAutoFixProposalResult* out,
                              const char* category,
                              const char* evidence,
                              const char* action,
                              int confidence)
{
    NxAutoFixProposalSuggestion* s;
    if (!out || out->suggestion_count >= 32) return;
    s = &out->suggestions[out->suggestion_count++];
    snprintf(s->category, sizeof(s->category), "%s", category ? category : "general");
    snprintf(s->evidence, sizeof(s->evidence), "%s", evidence ? evidence : "");
    snprintf(s->action, sizeof(s->action), "%s", action ? action : "Revisar manualmente el hallazgo.");
    s->confidence = confidence;
}

static int nx_read_file(const char* path, char* buffer, size_t buffer_size)
{
    FILE* fp;
    size_t n;
    if (!path || !buffer || buffer_size == 0) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    n = fread(buffer, 1, buffer_size - 1, fp);
    buffer[n] = 0;
    fclose(fp);
    return 1;
}

static void nx_generate_suggestions(const char* text, NxAutoFixProposalResult* out)
{
    if (nx_contains_ci(text, "implicit declaration") || nx_contains_ci(text, "declaration of function")) {
        nx_add_suggestion(out,
            "missing_include_or_prototype",
            "El log menciona una declaracion implicita de funcion.",
            "Agregar el header correcto o declarar el prototipo antes de usar la funcion. En C2x, las declaraciones implicitas deben tratarse como error.",
            92);
    }
    if (nx_contains_ci(text, "expected ';'") || nx_contains_ci(text, "before 'return'")) {
        nx_add_suggestion(out,
            "syntax_error",
            "El compilador reporta un error de sintaxis cerca de una sentencia return o un token esperado.",
            "Abrir el archivo y linea indicados por el log; revisar la instruccion anterior y agregar el punto y coma o delimitador faltante.",
            88);
    }
    if (nx_contains_ci(text, "unused variable") || nx_contains_ci(text, "warning:")) {
        nx_add_suggestion(out,
            "warning_cleanup",
            "El log contiene warnings de compilacion.",
            "Eliminar variables no usadas, inicializar datos o ajustar el codigo para que el warning no oculte problemas reales.",
            72);
    }
    if (nx_contains_ci(text, "tests failed") || nx_contains_ci(text, "***failed") || nx_contains_ci(text, "the following tests failed")) {
        nx_add_suggestion(out,
            "test_failure",
            "CTest reporta una o mas pruebas fallidas.",
            "Ejecutar la prueba fallida con --output-on-failure, identificar el comportamiento roto y agregar una prueba de regresion antes de corregir.",
            84);
    }
    if (nx_contains_ci(text, "no se reconoce") || nx_contains_ci(text, "not recognized") || nx_contains_ci(text, "commandnotfound")) {
        nx_add_suggestion(out,
            "missing_executable_or_path",
            "El sistema no encontro un ejecutable o comando esperado.",
            "Verificar que el target este integrado en CMake, que se haya compilado y que el script use la ruta correcta del ejecutable.",
            86);
    }
    if (out->suggestion_count == 0) {
        nx_add_suggestion(out,
            "manual_review",
            "No encontre patrones conocidos suficientes en el reporte.",
            "Revisar el log completo y ampliar el catalogo de patrones de Auto-Fix Proposal Engine.",
            40);
    }
}

static int nx_write_report(const char* path, const char* proposal_id, const char* source_report, const NxAutoFixProposalResult* out)
{
    FILE* fp;
    int i;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    fprintf(fp, "# NCOS Auto-Fix Proposal\n\n");
    fprintf(fp, "Proposal ID: %s\n\n", proposal_id ? proposal_id : "proposal");
    fprintf(fp, "Source analysis: %s\n\n", source_report ? source_report : "");
    fprintf(fp, "## Summary\n\n%s\n\n", out->summary);
    fprintf(fp, "## Suggested actions\n\n");
    for (i = 0; i < out->suggestion_count; ++i) {
        const NxAutoFixProposalSuggestion* s = &out->suggestions[i];
        fprintf(fp, "### %d. %s\n\n", i + 1, s->category);
        fprintf(fp, "Evidence: %s\n\n", s->evidence);
        fprintf(fp, "Action: %s\n\n", s->action);
        fprintf(fp, "Confidence: %d%%\n\n", s->confidence);
    }
    fprintf(fp, "## Safety policy\n\n");
    fprintf(fp, "This engine proposes fixes only. It must not modify source code automatically without human approval.\n");
    fclose(fp);
    return 1;
}

int NxAutoFixProposalEngine_CreateFromReport(const char* knowledge_root,
                                             const char* proposal_id,
                                             const char* analysis_report_path,
                                             NxAutoFixProposalResult* out)
{
    char text[32768];
    char dir[512];
    char safe[128];
    char filename[192];
    if (!out) return 0;
    nx_zero(out, sizeof(*out));
    if (!analysis_report_path || !*analysis_report_path) {
        snprintf(out->summary, sizeof(out->summary), "No se proporciono reporte de analisis.");
        return 0;
    }
    if (!nx_read_file(analysis_report_path, text, sizeof(text))) {
        snprintf(out->summary, sizeof(out->summary), "No se pudo leer el reporte de analisis: %s", analysis_report_path);
        return 0;
    }
    nx_generate_suggestions(text, out);
    out->success = out->suggestion_count > 0;
    snprintf(out->summary, sizeof(out->summary),
             "Se generaron %d propuesta(s) de correccion a partir del analisis de build.",
             out->suggestion_count);

    if (!nx_ensure_dirs(knowledge_root, dir, sizeof(dir))) return 0;
    nx_safe_id(safe, sizeof(safe), proposal_id);
    snprintf(filename, sizeof(filename), "%s.proposal.md", safe);
    if (!nx_join(out->proposal_path, sizeof(out->proposal_path), dir, filename)) return 0;
    return nx_write_report(out->proposal_path, safe, analysis_report_path, out);
}
