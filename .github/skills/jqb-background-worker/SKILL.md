---
name: jqb-background-worker
description: "Add non-blocking background jobs in JQB_WindowsLib using CreateThread, PostMessageW result handoff, progress signaling, and safe UI updates."
argument-hint: "Describe long-running task, progress granularity, and cancellation needs"
user-invocable: true
---
# JQB Background Worker

Implements background execution patterns that keep UI responsive.

## Use When
- A task can take more than a short interactive action.
- Scanning, exporting, large reads/writes, or long computations are needed.

## Procedure
1. Create worker context struct with input/output fields.
2. Start worker with CreateThread.
3. Send progress and completion to UI via PostMessageW.
4. Handle posted messages in window loop-safe handler.
5. Log worker state transitions with TextLogger.
6. Add cancel/stop flag when operation can be interrupted.

## Use Template
- [worker pattern template](./assets/worker_pattern.h.tpl)
- [message IDs template](./assets/worker_messages.h.tpl)

## Guardrails
- Do not call UI control methods directly from worker thread.
- Keep shared state synchronization simple and explicit.
