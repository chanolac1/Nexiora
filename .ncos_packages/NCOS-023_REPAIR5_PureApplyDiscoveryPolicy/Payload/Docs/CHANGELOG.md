# CHANGELOG

## 2026-07-10 — NCOS-023 Repair3

- Added deterministic pending-package discovery.
- Added staged Package Manager candidate build.
- Added safe manual bootstrap replacement for the final legacy installation.
- Synchronized all nine governing documents.
- Removed premature POST_BUILD self-replacement.
- Preserved transaction-specific rollback.


## NCOS-023 Repair5
- Added Pure Apply manifest eligibility policy.
- Legacy packages without `apply.*` metadata are excluded from automatic discovery.
- Added regression coverage for obsolete package selection.
