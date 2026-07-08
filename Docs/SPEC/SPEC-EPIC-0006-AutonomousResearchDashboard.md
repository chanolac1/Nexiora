# SPEC: EPIC-0006 Autonomous Research Dashboard

## Command

```powershell
nexiora research dashboard
```

## Output

Generates:

```text
Research/Sessions/first_autonomous_execution/dashboard.html
```

## Requirements

- Dashboard generation must be deterministic.
- Missing artifacts must be shown as missing, not treated as success.
- Runtime promotion must remain manual.
- The dashboard must be static and browser-openable.
