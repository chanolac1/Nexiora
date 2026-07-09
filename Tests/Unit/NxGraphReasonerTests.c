#include "Nexiora/NCOS/NxConceptGraph.h"
#include "Nexiora/NCOS/NxGraphReasoner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#define NX_MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define NX_MKDIR(path) mkdir(path, 0777)
#endif

static int fails = 0;
static void check(int cond, const char* msg)
{
    if (!cond)
    {
        printf("FAIL: %s\n", msg);
        fails++;
    }
}

int main(void)
{
    const char* root = "Build/ncos-005-test-root";
    NxConceptGraphEdge edge;
    NxGraphReasoningResult result;
    char output[4096];

    NX_MKDIR("Build");
    NX_MKDIR(root);

    check(NxConceptGraph_Link(root, "Genexus", "DataSelector", "related_to", "DataProvider", "test", 88, &edge), "link direct should succeed");
    check(NxConceptGraph_Link(root, "Genexus", "DataProvider", "belongs_to", "KnowledgeBase", "test", 82, &edge), "link indirect should succeed");

    check(NxGraphReasoner_Explain(root, "Genexus", "DataSelector", "DataProvider", &result), "direct reasoning should succeed");
    check(result.found_direct == 1, "direct relation should be detected");
    check(strstr(result.evidence, "dataselector") != NULL, "direct evidence should mention dataselector");
    check(result.confidence >= 80, "direct confidence should be high");

    memset(&result, 0, sizeof(result));
    check(NxGraphReasoner_Explain(root, "Genexus", "DataSelector", "KnowledgeBase", &result), "indirect reasoning should succeed");
    check(result.found_indirect == 1, "indirect relation should be detected");
    check(strstr(result.evidence, "dataprovider") != NULL, "indirect evidence should include intermediate concept");

    check(NxGraphReasoner_WhyRelated(root, "Genexus", "DataSelector", "KnowledgeBase", output, sizeof(output)), "why-related should succeed");
    check(strstr(output, "Razonamiento sobre grafo") != NULL, "output should include graph reasoning header");
    check(strstr(output, "Evidencia usada") != NULL, "output should include evidence section");

    if (fails != 0) return 1;
    return 0;
}
