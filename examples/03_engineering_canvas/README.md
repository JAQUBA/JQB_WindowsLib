# Example Project 03 - Engineering Canvas

Runnable multi-file engineering-style desktop app.

## Features

- `AppState` + `AppUI` split
- custom `CanvasWindow` subclass
- right-side `TreePanel` visibility controls
- debounced preview refresh (`TimerUtils`)
- background generation task (`CreateThread`)

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
- `src/Canvas/EngineeringCanvas.h/.cpp`
