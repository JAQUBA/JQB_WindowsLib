---
name: jqb-tab-feature-pack
description: "Add a new tab or section in JQB_WindowsLib apps with card layout, section headers, field labels, callback wiring, and theme-consistent primary/secondary actions."
argument-hint: "Describe tab goal, controls needed, and data bindings"
user-invocable: true
---
# JQB Tab Feature Pack

Implements one complete tab/section quickly with consistent architecture.

## Use When
- Expanding an existing application with a new functional area.
- Reworking legacy tab layout to match current UI guide.

## Procedure
1. Define tab intent and user operations.
2. Build card blocks using [tab layout checklist](./assets/tab-layout-checklist.md).
3. Place controls and wire callbacks via AppState actions.
4. Style one primary action with accent button style.
5. Add live status line and optional progress indicator.
6. Add persistence keys if tab contains user-adjustable settings.

## Acceptance Criteria
- UI remains readable at common desktop sizes.
- No protocol-heavy logic in direct UI handlers.
- Main user path in tab can be executed in under 3 clicks.
