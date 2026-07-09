#include "Nexiora/NCOS/NxConceptGraph.h"

#include <stdio.h>
#include <string.h>

static int fails = 0;

static void check(int cond, const char* message)
{
    if (!cond)
    {
        printf("FAIL: %s\n", message);
        fails++;
    }
}

int main(void)
{
    const char* root = "Build/ncos004_concept_graph_test_root";
    NxConceptGraphEdge edge;
    NxConceptGraphEdge found;
    NxConceptGraphStats stats;
    char out[4096];
    char norm[128];

    NxConceptGraph_Normalize("Data Selector", norm, sizeof(norm));
    check(strcmp(norm, "data_selector") == 0, "normalize should create stable identity");

    check(NxConceptGraph_Link(root, "GeneXus", "DataSelector", "related_to", "DataProvider", "unit-test", 88, &edge), "link should succeed");
    check(strcmp(edge.domain, "genexus") == 0, "domain should be normalized");
    check(strcmp(edge.from, "dataselector") == 0, "from should be normalized");
    check(strcmp(edge.relation, "related_to") == 0, "relation should be normalized");
    check(strcmp(edge.to, "dataprovider") == 0, "to should be normalized");
    check(edge.confidence == 88, "confidence should be preserved");
    check(strstr(edge.path, "edges.jsonl") != NULL, "edge should expose path");

    check(NxConceptGraph_Find(root, "GeneXus", "DataSelector", "related_to", "DataProvider", &found), "find should locate persisted edge");
    check(strcmp(found.source, "unit-test") == 0, "find should preserve source");

    check(NxConceptGraph_Link(root, "GeneXus", "DataSelector", "used_by", "Procedure", "unit-test", 77, NULL), "second link should succeed");
    check(NxConceptGraph_Stats(root, "GeneXus", "DataSelector", &stats), "stats should succeed");
    check(stats.edge_count >= 2, "stats should count edges");
    check(stats.outgoing_count >= 2, "stats should count outgoing edges");

    check(NxConceptGraph_Show(root, "GeneXus", "DataSelector", out, sizeof(out)), "show should succeed");
    check(strstr(out, "dataselector") != NULL, "show should include concept");
    check(strstr(out, "dataprovider") != NULL, "show should include linked concept");
    check(strstr(out, "procedure") != NULL, "show should include second linked concept");

    if (fails != 0) return 1;
    printf("NxConceptGraphTests passed.\n");
    return 0;
}
