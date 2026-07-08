# EPIC-0002: Knowledge Engine Foundation

## Summary
EPIC-0002 introduces the first Knowledge Engine layer for Nexiora.

The Research Lab can now convert experimental evidence and graph relationships into explicit knowledge facts. These facts can be queried by subject, predicate, and confidence threshold.

## Added

- `NxKnowledgeBase`
- `NxKnowledgeQuery`
- `NxKnowledgeBaseTests`
- `NxKnowledgeQueryTests`
- RFC-0029
- SPEC-EPIC-0002

## Philosophy
The Knowledge Engine is advisory. It may support recommendations, but it must never promote code to Runtime automatically.

Nexiora may recommend. Only a human may promote.
