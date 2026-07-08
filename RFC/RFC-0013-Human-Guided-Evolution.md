# RFC-0013: Human Guided Evolution

Status: Accepted
Version: 0.0.7

## Principle

Nexiora may propose improvements, run experiments, and measure evidence, but stable promotion requires explicit human approval.

## Rationale

Autonomous optimization without governance can degrade correctness, security, or maintainability.
Human approval keeps the project aligned with long-term engineering goals.

## Requirements

- Candidate improvements must not automatically replace stable baselines.
- Evidence reports must be generated before approval.
- Approval records must be stored under `Benchmarks/Approvals`.
- Rejected changes must not become the stable baseline.
