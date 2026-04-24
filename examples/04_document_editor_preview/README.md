# Example Project 04 - Document Editor Preview

Runnable project with document editor on the left and live canvas preview on the right.

## Features

- RichEdit-based multiline document editor
- debounced preview render on text change
- custom `CanvasWindow` preview renderer
- export current editor content to file

## Build

```bash
pio run
```

## Run

```bash
pio run --target exec
```

## Files

- `src/main.cpp`
- `src/AppState.h/.cpp`
- `src/AppUI.h/.cpp`
- `src/Canvas/DocumentCanvas.h/.cpp`
