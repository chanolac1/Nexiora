# SPEC: EPIC-0001 Autonomous Laboratory Foundation

## Modules

### NxResearchQueue
A fixed-capacity in-memory queue of research work items. Selection chooses the highest-priority pending item and uses the lowest id as a deterministic tie-breaker.

### NxResearchPolicy
Evaluates whether an item should run, be skipped, or stop the current session. The default policy is conservative and bounded.

### NxResearchSession
Tracks cycles, executed experiments, successes, failures, and skipped items.

### NxAutonomousResearchLoop
Runs a bounded number of cycles. It selects queue items, evaluates policy, invokes execution/evidence/journal/graph callbacks, updates queue state, and returns a session result.

## Safety invariant
The loop never promotes runtime changes. It only executes callbacks and returns a result for human review.

## Verification
EPIC-0001 adds tests for queue ordering, policy decisions, session counters, and full loop execution.
