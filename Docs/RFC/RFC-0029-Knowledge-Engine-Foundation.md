# RFC-0029: Knowledge Engine Foundation

## Status
Accepted

## Context
Nexiora already records experiments, evidence, sessions, promotions, and graph relationships. The next architectural step is to convert these records into explicit, queryable knowledge facts.

## Decision
Introduce a small Knowledge Engine foundation composed of:

- `NxKnowledgeBase`: stores evidence-backed facts.
- `NxKnowledgeQuery`: performs bounded, allocation-free queries over stored facts.

Facts are represented as subject-predicate-object triples with confidence and an optional source graph node.

## Rule
Knowledge may support recommendations, but it must not promote Runtime changes. Promotion remains a human decision.

## Consequences
The laboratory can begin answering questions such as:

- What does Nexiora know about this experiment?
- Which facts are supported by evidence?
- Which facts have confidence above a threshold?
- Which graph node produced a fact?
