# CHANGELOG

## CAPABILITY-02 Sprint-03 Repair2
- Corrected contextual evidence selection for conversational follow-up questions.
- Corrected the conversational demonstration evidence dataset.
- Added regression coverage against repeated answers across semantically different turns.

- Fixed conversational reasoning leakage by separating focal and comparative evidence selection.
- Added regression requiring the first answer to exclude semantic-memory evidence and use one top-ranked chunk.

## NCOS-021
- Added native `apply` workflow to Package Manager.
- Added configure, full build, zero-warning gate, CTest and documentation validation phases.
- Added exact transaction rollback after post-install failures.
- Removed external scripts from the official package application contract.
