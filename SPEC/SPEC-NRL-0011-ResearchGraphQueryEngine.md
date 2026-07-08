# SPEC — NRL-0011 Research Graph Query Engine

## Purpose
Provide read-only graph queries for Nexiora Research Lab.

## Functions

- `NxResearchGraphQuery_CountNodesByType`
- `NxResearchGraphQuery_FindNodesByType`
- `NxResearchGraphQuery_FindOutgoingEdges`
- `NxResearchGraphQuery_FindIncomingEdges`
- `NxResearchGraphQuery_FindConnectedNodes`

## Status model

- `OK`: Query completed and all matches fit in the caller buffer.
- `INVALID_ARGUMENT`: Required pointer missing or inconsistent buffer arguments.
- `NOT_FOUND`: Requested source node does not exist.
- `BUFFER_TOO_SMALL`: Query found more matches than the output buffer can hold. `total_matches` still reports the real match count and `written_count` reports how many were written.

## Wildcards

- `NX_RESEARCH_NODE_UNKNOWN` matches all node types.
- `NX_RESEARCH_EDGE_UNKNOWN` matches all edge types.

## Ownership
The query engine returns pointers into the graph. The caller must not free them and must not use them after the graph is cleared or shut down.
