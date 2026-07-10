---
name: jqb-skill-evolution
description: "Continuously update existing Copilot skills with new project knowledge, proven patterns, and post-task lessons so the skill set improves over time without duplication."
argument-hint: "Describe new lessons, source project, and which existing skills should be updated"
user-invocable: false
---
# JQB Skill Evolution

Deprecated for manual slash use. Prefer `/learn`.

Maintains and improves workspace skills as new knowledge appears in real projects.

## Use When
- A finished task revealed a reusable pattern.
- A bugfix exposed missing guidance in an existing skill.
- A project introduced a new architecture variant worth codifying.
- Existing skills became outdated or duplicated.

## Procedure
1. Capture new insight with context:
   - source project/module
   - problem and root cause
   - final pattern that worked
2. Map insight to target skill(s):
   - update existing skill first
   - create a new skill only if no existing scope fits
3. Apply changes consistently:
   - SKILL.md description and procedure
   - assets/references templates if needed
   - skill index documentation
4. Record evolution entry using [knowledge delta log](./assets/knowledge-delta-log.md).
5. Validate quality gates before merge.

## Quality Gates
1. No duplicate skill purpose across files.
2. Description includes clear trigger phrases ("Use when...").
3. Workflow remains procedural and actionable.
4. Related docs/index entries are updated in same change.
5. Old guidance is removed or marked obsolete when replaced.

## Output Format
1. Knowledge delta summary.
2. Skills updated and why.
3. Files changed.
4. Follow-up improvements backlog.
