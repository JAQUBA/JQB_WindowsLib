---
name: jqb-doc-sync
description: "Synchronize implementation and documentation in JQB_WindowsLib projects by updating README, docs index, component pages, examples list, and migration notes in the same change."
argument-hint: "Describe what changed in code and what docs must be aligned"
user-invocable: true
---
# JQB Documentation Sync

Ensures code and docs evolve together.

## Use When
- New modules/components are added.
- Public API behavior changes.
- Examples are added/renamed.

## Procedure
1. Build docs impact map with [docs impact checklist](./assets/docs-impact-checklist.md).
2. Update README summary and quick links.
3. Update docs/README.md index if new docs exist.
4. Update specific docs pages for changed behavior or APIs.
5. Add migration notes for non-backward-compatible behavior.

## Output
- Changed docs files.
- Summary of new/changed sections.
- Open documentation debt items.
