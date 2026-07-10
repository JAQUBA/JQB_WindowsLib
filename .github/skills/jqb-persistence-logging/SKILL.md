---
name: jqb-persistence-logging
description: "Implement robust settings persistence and operational logging in JQB_WindowsLib using ConfigManager, TextLogger, log filtering, and export-ready snapshots."
argument-hint: "List settings keys, log categories, and retention/export expectations"
user-invocable: true
---
# JQB Persistence and Logging

Builds stable app state storage and operator-visible logs.

## Use When
- Adding configurable UI options.
- Building diagnostics and supportability paths.
- Preparing app for field deployments.

## Procedure
1. Define settings map with defaults using [settings key template](./assets/settings-keys-template.md).
2. Keep AppState::load() and AppState::save() symmetric.
3. Attach TextLogger to TextArea once UI is ready.
4. Emit INFO/ERROR for business events and TX/RX for protocol frames.
5. Add snapshot export action for support workflows.

## Acceptance Criteria
- Restart preserves all configured values.
- Logs are readable, filterable, and exportable.
- Missing/invalid settings fall back safely to defaults.
