# DECISIONS

## ADR — Session storage owns parent-directory creation

Conversational persistence must create its own parent directories transactionally before opening a session file. User workflows must not require manual directory preparation.
