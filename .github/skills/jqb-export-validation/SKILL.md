---
name: jqb-export-validation
description: "Harden export workflows with pre-export validation, machine bounds checks, format sanity checks, and release-ready output diagnostics for desktop engineering tools."
argument-hint: "Describe export format, safety constraints, and target machine/runtime"
user-invocable: true
---
# JQB Export Validation

Adds guardrails before generating production output files.

## Use When
- Application exports machine instructions or critical artifacts.
- Output can damage hardware if invalid.
- You need release confidence and traceability.

## Procedure
1. Add configuration validation using [validation matrix template](./assets/validation-matrix-template.md).
2. Validate geometry/range constraints before export.
3. Validate generated output structure and required commands/tokens.
4. Emit concise export summary (counts, duration estimate, warnings).
5. Add failure messages actionable for operator.

## Acceptance Criteria
- Invalid configs are blocked before file writing.
- Bounds and safety checks are explicit and logged.
- Export summary is reproducible and useful in support/debugging.
