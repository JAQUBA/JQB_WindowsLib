---
name: jqb-protocol-contract
description: "Design and evolve binary/structured protocol contracts for desktop-device communication with report IDs, field offsets, size guarantees, and migration-safe changes."
argument-hint: "Provide existing reports, required new fields, and backward compatibility constraints"
user-invocable: true
---
# JQB Protocol Contract

Creates stable communication contracts between host app and device.

## Use When
- Introducing new report fields.
- Adding feature flags and capabilities.
- Refactoring protocol without breaking deployed devices.

## Procedure
1. Build report catalog with [report table template](./assets/report-table-template.md).
2. Reserve and document report IDs and field offsets.
3. Keep wire sizes fixed where possible.
4. Add version/capability report for negotiation.
5. Define behavior for unknown IDs and short payloads.

## Validation
- Pack/unpack tests pass for all report variants.
- Legacy device path remains usable.
- Protocol doc and source constants remain synchronized.
