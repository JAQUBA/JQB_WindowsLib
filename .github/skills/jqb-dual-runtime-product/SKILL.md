---
name: jqb-dual-runtime-product
description: "Build products that include embedded firmware and Windows desktop app together: shared protocol contracts, versioning, compatibility fallback, and coordinated release flow."
argument-hint: "Describe firmware target, transport (HID/Serial), and configurator capabilities"
user-invocable: true
---
# JQB Dual Runtime Product

Designs and implements a firmware + desktop application workflow as one coherent product.

## Use When
- Firmware and Windows tool are developed in parallel.
- You need shared protocol headers and capability discovery.
- Backward compatibility between app and old firmware matters.

## Procedure
1. Define protocol ownership and transport responsibilities.
2. Create a shared constants header using [contract header template](./assets/protocol_contract.h.tpl).
3. Add capability/version report for host-side feature detection.
4. Implement host fallback logic for missing capability reports.
5. Add release checklist with synchronized firmware/app version notes.

## Acceptance Criteria
- Host can detect device capability/version.
- Unknown report or old firmware paths degrade gracefully.
- Firmware and app docs show consistent field definitions.
