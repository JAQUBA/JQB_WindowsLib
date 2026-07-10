---
name: JQB Protocol and Pipeline Rules
description: "Use when editing shared protocol headers, staged processing pipelines, or export modules. Enforces explicit stage boundaries, versioned contracts, and deterministic outputs."
applyTo:
  - "**/protocol.h"
  - "**/shared/protocol.h"
  - "**/Pipeline/**/*.h"
  - "**/Pipeline/**/*.cpp"
  - "**/GCode/**/*.h"
  - "**/GCode/**/*.cpp"
---
# JQB Protocol and Pipeline Rules

- Treat protocol constants as contract: stable IDs, explicit sizes, and documented field offsets.
- Add capability/version negotiation for new optional behaviors.
- Keep pipeline stages explicit (detect, parse, normalize, generate, optimize, export).
- Keep stage I/O structures clear and serializable where possible.
- Make export deterministic for same input + config.
- Separate parse-time errors from generation/export-time errors.
- Add pre-export validation gates for bounds/safety constraints.
