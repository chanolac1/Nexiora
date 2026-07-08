# SPEC: EPIC-0002 Knowledge Engine Foundation

## Goal
Create the first stable Knowledge Engine layer for Nexiora Research Lab.

## Components

### NxKnowledgeBase
Stores facts as triples:

```text
subject --predicate--> object
```

Each fact includes:

- numeric id,
- subject,
- predicate,
- object,
- confidence from 0 to 100,
- optional source graph node id.

### NxKnowledgeQuery
Provides bounded queries using caller-owned result buffers:

- query by subject,
- query by predicate,
- query by minimum confidence.

## Constraints

- No automatic Runtime promotion.
- No hidden allocation inside query results.
- No dependency from Runtime to Research.
- Evidence and graph nodes remain traceable.

## Tests

- fact insertion,
- duplicate rejection,
- confidence validation,
- graph source validation,
- query by subject,
- query by predicate,
- query by confidence,
- bounded result handling.
