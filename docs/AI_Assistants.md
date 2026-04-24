# AI Assistants Guide (GitHub Copilot + Claude Code)

This guide helps you use AI coding assistants effectively with JQB_WindowsLib.
It is tuned for practical desktop app development on Windows.

## Goals

- reduce back-and-forth while implementing UI + I/O features
- keep code consistent with WinAPI and MinGW compatibility constraints
- speed up changes without breaking architecture

## Prerequisites

Before prompting, make sure the assistant can read:

- [README.md](../README.md)
- [docs/QuickStart.md](QuickStart.md)
- [docs/ProjectBlueprint.md](ProjectBlueprint.md)
- [docs/examples/README.md](examples/README.md)
- [docs/examples/08_engineering_canvas_app.md](examples/08_engineering_canvas_app.md)
- [docs/examples/09_document_editor_canvas.md](examples/09_document_editor_canvas.md)
- [docs/LogWindow.md](LogWindow.md)
- [docs/TreePanel.md](TreePanel.md)

For GitHub Copilot, also ensure:

- [/.github/copilot-instructions.md](../.github/copilot-instructions.md)

For Claude Code, also ensure:

- [../CLAUDE.md](../CLAUDE.md)

## Prompt Template (Feature Work)

Use this structure when asking for non-trivial changes:

```text
Task:
Implement [feature] in [project/repo].

Context:
- This is a C++17 Windows app using JQB_WindowsLib.
- Architecture follows setup()/loop(), AppState/AppUI split, and callback-driven UI.
- Relevant files: [list files].

Requirements:
1. [behavior requirement]
2. [data model requirement]
3. [UI requirement]
4. [error handling requirement]
5. [persistence requirement if needed]

Constraints:
- Use explicit WinAPI wide calls (CreateWindowExW, MessageBoxW, etc.).
- No std::thread (use CreateThread if needed).
- Keep code MinGW-friendly.
- Do not refactor unrelated modules.

Validation:
- Build passes.
- Existing behavior unchanged.
- Include short manual test checklist.
```

## Prompt Template (Refactor)

```text
Refactor request:
Refactor [module/file] for readability and maintainability.

Keep unchanged:
- public API
- behavior
- file format compatibility

Improve:
- naming
- function size
- comment quality
- separation of responsibilities

Output:
- code edits
- concise change summary
- risk list
```

## Prompt Template (Bug Fix)

```text
Bug:
[describe observed behavior]

Expected:
[correct behavior]

Repro steps:
1. ...
2. ...
3. ...

Scope:
Only fix files directly related to [module].

Deliver:
- root cause
- patch
- regression risks
- tests/checklist
```

## High-Leverage Requests for JQB_WindowsLib Apps

These requests are especially useful in projects similar to gerber2gcode and WektoroweLitery2:

1. Generate AppState/AppUI split for a new tool.
2. Add debounced auto-refresh for InputField callbacks.
3. Add custom CanvasWindow subclass and drawing pipeline.
4. Add ListBox + TreePanel integration for layered visibility controls.
5. Add background worker with CreateThread and progress/log updates.
6. Add ConfigManager persistence for all user settings.
7. Add keyboard shortcuts via subclassed window procedure.

## Review Checklist for AI Output

Use this checklist before accepting generated code:

1. Window lifecycle: `setup()` creates and initializes exactly one `SimpleWindow`.
2. Encoding: Label text is wide (`L"..."`), other control text is UTF-8 (`"..."`).
3. Compatibility: no `std::thread`, no unsupported APIs for MinGW compatibility targets.
4. Thread safety: UI updates from worker threads are handled consistently.
5. Persistence: settings read and write keys are symmetric.
6. Memory ownership: components added to `SimpleWindow` are not deleted manually.
7. Scope control: no unrelated refactors.

## Example Prompt: New Engineering Tool Skeleton

```text
Create a desktop tool skeleton with JQB_WindowsLib.

Requirements:
- Files: main.cpp, AppState.h/.cpp, AppUI.h/.cpp, Canvas/MyCanvas.h/.cpp.
- setup() should initialize window, create UI, create canvas, load settings.
- Include TextArea log area and ProgressBar.
- Include one background action using CreateThread.
- Include debounced InputField callback (400 ms) using TimerUtils.
- Keep code C++17 and MinGW-compatible.

Also:
- update README with architecture section
- add docs/examples markdown for this skeleton
```

## Example Prompt: Claude Code Task Pack

```text
You are editing a C++17 Windows desktop app using JQB_WindowsLib.

Do the following in one pass:
1) Add a right-side layer panel with TreePanel.
2) Add a custom canvas overlay toggle.
3) Persist panel and toggle state in ConfigManager.
4) Add keyboard shortcuts (Ctrl+R reload, F6 fit).
5) Update README and docs/examples/README links.

Constraints:
- Keep existing style.
- Use explicit wide WinAPI APIs.
- No unrelated formatting changes.

After edits:
- report changed files
- report any remaining risks
```

## Keep Docs in Sync

When a feature changes behavior, update docs in the same change set:

- [README.md](../README.md)
- [docs/README.md](README.md)
- [docs/examples/README.md](examples/README.md)
- relevant component docs in `docs/`
