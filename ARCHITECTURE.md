# Nexiora Architecture

Nexiora is organized as a research-first platform. Runtime code is protected; research code may observe, measure, compare, and recommend, but it must not promote changes without human approval.

## Architectural layers

1. **NCP Core Platform**: memory, logging, strings, containers, benchmark primitives, evidence primitives, and runtime services.
2. **Research Lab**: experiments, manifests, registry, journal, evidence, promotion, scheduler, graph, queue, policies, sessions, and autonomous loop.
3. **Knowledge Layer**: graph exports and graph queries used to explain why a recommendation exists.
4. **Human Promotion Boundary**: the system may recommend; only a human may approve promotion to Runtime.

## Dependency rules

- Runtime must not depend on Research Lab.
- Research Lab may depend on NCP.
- Promotion may read evidence, but promotion must not modify runtime code.
- Graph and Journal are audit systems; they must preserve reasoning and evidence.
- Autonomous components must terminate in recommendation or session result, never automatic deployment.

## Autonomous Laboratory Foundation

EPIC-0001 introduces the first durable loop structure:

```text
Research Queue
    -> Research Policy
    -> Autonomous Research Loop
    -> Evidence / Journal / Graph callbacks
    -> Session summary
    -> Human review boundary
```

This architecture prepares Nexiora for future persistent laboratory state, CLI command integration, and AI-assisted hypothesis generation.
