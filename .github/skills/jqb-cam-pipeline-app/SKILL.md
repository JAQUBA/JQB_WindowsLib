---
name: jqb-cam-pipeline-app
description: "Build CAM-style desktop applications with staged parse-normalize-generate-export pipeline, canvas preview, path optimization, and robust G-Code generation."
argument-hint: "Describe input formats, pipeline stages, and export target requirements"
user-invocable: true
---
# JQB CAM Pipeline App

Creates deterministic multi-stage processing architecture for CNC/CAM desktop tools.

## Use When
- Input file parsing and geometry processing are non-trivial.
- You need live preview + export generation.
- You need reproducible output and controllable performance.

## Procedure
1. Define explicit pipeline stages using [pipeline stage template](./assets/pipeline-stage-template.md).
2. Split parse-only stage from generate/export stage.
3. Keep intermediate data objects for preview and debugging.
4. Add ordering optimization stage for runtime efficiency.
5. Validate bounds/safety before export.
6. Add route/time estimate in export summary.

## Acceptance Criteria
- Same input + config yields deterministic export.
- Parse errors and generation errors are separately reported.
- Canvas preview and final export are driven by same normalized data model.
