---
name: learn
description: "Capture lessons learned after completed tasks and update Copilot skills, templates, and docs with proven patterns in a consistent, non-duplicated way."
argument-hint: "Describe what was learned, from which task/project, and what should be updated"
user-invocable: true
---
# Learn

Manual workflow for continuous improvement of workspace skills and guidance.

## Use When
- You finished implementation and have reusable lessons.
- A fix uncovered missing guidance in existing skills.
- A pattern should be codified for future projects.

## Procedure
1. Capture the lesson using [knowledge delta log](./assets/knowledge-delta-log.md).
2. Map lesson to existing skills first (avoid duplicates).
3. Update relevant SKILL.md and assets/references.
4. Update docs/index files in same change set.
5. Remove or mark obsolete guidance that is superseded.

## Quality Gates
1. No duplicate skill purpose.
2. Trigger phrases are clear in descriptions.
3. Procedure remains concrete and actionable.
4. Docs and indexes stay synchronized.

## Output
1. Lesson summary.
2. Files updated.
3. Skills changed.
4. Follow-up backlog.
