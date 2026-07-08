# SPEC-0002 — Manifest and Registry Engine

Defines experiment manifests (`experiment.nx`) and registry index (`registry.nxr`).

## Requirements
- Manifest roundtrip must preserve experiment identity, module, status and hypothesis.
- Registry must support add, write, read and find by experiment id.
- Registry is an index, not the canonical experiment body.
