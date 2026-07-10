---
name: jqb-project-archetypes
description: "Select the best starter architecture archetype for a new project (firmware+configurator, audio analyzer, or CAM pipeline), then generate implementation plan and starter file map."
argument-hint: "Describe product goal, inputs/outputs, and hardware/protocol constraints"
user-invocable: true
---
# JQB Project Archetypes

Helps pick and instantiate the right architecture family before coding.

## Use When
- Starting a new project and deciding baseline architecture.
- Migrating a prototype into production-friendly structure.

## Procedure
1. Match project to archetype matrix in [archetype matrix](./assets/archetype-matrix.md).
2. Produce file/module map from selected archetype.
3. Recommend exact follow-up skills and agents.
4. Produce phased implementation plan (MVP -> stabilization -> docs).

## Output Format
1. Selected archetype and rationale.
2. Suggested module/file skeleton.
3. Skill/agent invocation sequence.
4. Risks and validation checklist.
