# Project Blueprint for Production Apps

This blueprint is derived from two real projects built on JQB_WindowsLib:

- gerber2gcode (pipeline-heavy engineering CAM app)
- WektoroweLitery2 (document editor + preview + export)

Use it as a starting point for apps bigger than simple demos.

## Recommended Folder Layout

```text
src/
├── main.cpp                # setup()/loop() only, minimal orchestration
├── AppState.h / .cpp       # global state, settings, shared actions
├── AppUI.h / .cpp          # UI creation, layout, style helpers
├── Canvas/
│   └── MyCanvas.h / .cpp   # CanvasWindow subclass for custom rendering
├── Pipeline/
│   └── Pipeline.h / .cpp   # parse/compute/generate workflow
├── Export/
│   └── Exporter.h / .cpp   # file output layer
└── Domain/
    └── *.h / *.cpp         # domain logic and data models

docs/
├── README.md
├── AI_Assistants.md
└── examples/
```

## Main Architectural Pattern

### 1. Keep `main.cpp` thin

`main.cpp` should only:

1. initialize window and app-level services
2. call `createUI(...)`
3. create canvas
4. load settings
5. register onClose save/cleanup

Avoid business logic in `main.cpp`.

### 2. Centralize app state in `AppState`

Store shared pointers and app flags in one place:

- `SimpleWindow*`, canvas, log widgets, progress widgets
- InputField and CheckBox pointers
- loaded data model
- persistent settings values

Put shared actions in `AppState.cpp`:

- `doLoad...()`
- `doGenerate...()`
- `doExport...()`
- `logMsg(...)`
- `loadSettings()` / `saveSettings()`

### 3. Keep `AppUI` focused on composition

In `AppUI.cpp`:

- build controls
- set callbacks
- define style helpers (`styleBtn`, `addLabel`, etc.)
- handle relayout and resize plumbing

Do not place domain algorithms in UI callbacks.

### 4. Move heavy logic into `Pipeline` modules

Pipeline functions should be testable without UI:

- parse inputs
- validate
- compute
- generate outputs

UI actions should call pipeline and display results.

### 5. Use custom `CanvasWindow` for visual tools

For preview-heavy apps, subclass `CanvasWindow`:

- hold domain data pointers
- implement `onDraw(HDC, RECT)`
- add helper methods like `zoomToFit`, `setDocument`, `clearData`

This keeps rendering code isolated from form logic.

## Long-Running Tasks Pattern

Use `CreateThread` for expensive operations.

1. collect immutable inputs from UI
2. set `running` flag
3. show `ProgressBar` marquee or status text
4. run pipeline in worker thread
5. append logs and update final state
6. clear `running` flag

Guard against concurrent runs with one shared boolean flag.

## Debounced Auto-Refresh Pattern

For expensive recompute on text input:

1. in `InputField` callback, call debounce helper
2. restart timer on each keystroke (`TimerUtils::restartDebounceTimer`)
3. recompute only when timer fires

Recommended delay for engineering tools: 250-500 ms.

## Layered UI and Side Panels

For complex visualization tools:

- use a right-side LISTBOX + `TreePanel` for toggles
- keep section expand/collapse flags in `AppState`
- on click: update flag, rebuild panel, redraw canvas

This pattern scales much better than dozens of standalone checkboxes.

## Settings and Persistence

Use `ConfigManager` consistently:

- read all keys in `loadSettings()`
- write all keys in `saveSettings()`
- use identical key names for read/write

If a control value is user-facing, persist it unless intentionally transient.

## Logging Strategy

Use one logging facade (`logMsg`) and one sink:

- `TextArea` for embedded logs
- `LogWindow` for detached logs

Do not print logs from random modules directly to UI controls.

## Suggested Implementation Order

1. Create minimal window and controls.
2. Add AppState + settings persistence.
3. Add custom canvas and first render path.
4. Add pipeline parse/compute functions.
5. Add background worker for expensive actions.
6. Add layer/toggle side panel.
7. Add export path and final UX polish.

## Anti-Patterns to Avoid

1. Large `setup()` with business logic mixed into UI code.
2. Duplicated state in many files.
3. Expensive recompute in every `EN_CHANGE` without debounce.
4. Manual memory management for components already owned by `SimpleWindow`.
5. Silent config key drift (read key differs from write key).

## Reference Examples

- [examples/08_engineering_canvas_app.md](examples/08_engineering_canvas_app.md)
- [examples/09_document_editor_canvas.md](examples/09_document_editor_canvas.md)
