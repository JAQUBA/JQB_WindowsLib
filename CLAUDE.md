# CLAUDE.md - JQB_WindowsLib Project Instructions

This file provides repository-specific instructions for Claude Code when working in JQB_WindowsLib.

## Project Summary

JQB_WindowsLib is a C++17 native Windows UI library built on WinAPI.
It targets practical desktop tools with simple lifecycle functions (`init`, `setup`, `loop`).

## Core Constraints

1. Use C++17.
2. Keep MinGW compatibility in mind.
3. Prefer explicit wide WinAPI calls (`CreateWindowExW`, `MessageBoxW`, `CreateFontW`).
4. Do not use `std::thread`; use `CreateThread` where background work is needed.
5. Use `jqb_compat::to_wstring` where needed for MinGW compatibility.
6. Include `Core.h` first in application source files.

## Architectural Rules

1. Keep `main.cpp` thin.
2. Place shared pointers and actions in `AppState`.
3. Keep control creation/layout in `AppUI`.
4. Keep heavy logic in domain or pipeline modules.
5. For preview-heavy tools, use a `CanvasWindow` subclass.

## UI and Ownership Rules

1. `SimpleWindow` is singleton-like; do not create multiple main windows.
2. Components added via `window->add(...)` are owned by `SimpleWindow`.
3. Do not manually delete components owned by `SimpleWindow`.
4. For additional windows use `OverlayWindow`, `LogWindow`, or raw WinAPI windows.

## Text and Encoding Rules

1. `Label` text uses wide strings (`L"..."`).
2. Many other controls use UTF-8 `char*` text.
3. Use `StringUtils::utf8ToWide` and `StringUtils::wideToUtf8` for conversions.

## Documentation Sync Rules

When adding features or changing behavior, update docs in the same change:

- `README.md`
- `docs/README.md`
- relevant docs in `docs/`
- `docs/examples/README.md` if examples change

## Preferred Patterns

- Debounced input handling: `TimerUtils::restartDebounceTimer(...)`
- Layered toggle panels: `TreePanel`
- Detached logs: `LogWindow`
- Persistent settings: `ConfigManager`
- Background tasks: `CreateThread` + progress/log feedback
- Periodic polling: `PollingManager` (named groups, `tick()` from `loop()`)
- App-wide log sink: `TextLogger` attached to a `TextArea` (with INFO/ERROR/TX/RX filters)
- Industrial protocols: `IO/Modbus` (`ModbusSerialPort` + `RtuMaster`) — use this instead of generic `Serial` when DCB control is needed
- Modern look: `applyTheme(window, Theme::catppuccinMocha())` + `styleAccentButton()` for primary actions

## Useful References

- `README.md`
- `docs/README.md`
- `docs/UIDesignGuide.md` — **prescriptive UI layout rules**: cards, sections, accent buttons, status dot, anti-patterns
- `docs/ProjectBlueprint.md`
- `docs/AI_Assistants.md`
- `docs/examples/08_engineering_canvas_app.md`
- `docs/examples/09_document_editor_canvas.md`
- `.github/copilot-instructions.md`

## Change Quality Expectations

When editing code:

1. Keep style and formatting consistent with nearby code.
2. Avoid unrelated refactors.
3. Preserve existing public APIs unless asked otherwise.
4. Provide concise rationale for non-obvious changes.
5. Call out risks and validation steps after edits.
