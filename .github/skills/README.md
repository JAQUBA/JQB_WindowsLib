# JQB Copilot Skills

This folder contains reusable Copilot skills for future JQB_WindowsLib projects.

## Available Skills

- `/jqb-app-skeleton` : build initial project skeleton with AppState/AppUI split.
- `/jqb-tab-feature-pack` : add one complete tab or feature section.
- `/jqb-io-pipeline` : integrate Serial/BLE/HID/Modbus/Audio communication pipelines.
- `/jqb-background-worker` : add CreateThread-based long-running job pattern.
- `/jqb-persistence-logging` : add ConfigManager + TextLogger workflow.
- `/jqb-doc-sync` : synchronize docs after code changes.
- `/jqb-dual-runtime-product` : coordinate firmware + Windows app product workflows.
- `/jqb-protocol-contract` : design and evolve host-device report contracts safely.
- `/jqb-audio-analysis-app` : scaffold audio generator/analyzer application patterns.
- `/jqb-cam-pipeline-app` : build staged parse/normalize/generate/export CAM pipelines.
- `/jqb-export-validation` : add hard validation gates before output export.
- `/jqb-project-archetypes` : pick the right project family and rollout plan before implementation.
- `/learn` : continuously update skills with lessons learned and new proven patterns.

## Notes

- Skill names match folder names and are slash-invocable.
- Each skill includes references or templates under assets/ or references/.
- Skills are designed to minimize startup time in new tool projects.
