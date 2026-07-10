## Architecture

- Entry: setup()/loop() in src/main.cpp
- Shared state and services: src/app/AppState.h/.cpp
- UI layout and callbacks: src/ui/AppUI.h/.cpp
- Optional protocol/domain modules: src/io/* and src/domain/*

## Runtime Flow

1. setup() loads config and builds UI.
2. UI binds callbacks to AppState actions.
3. loop() drives PollingManager and lightweight UI-side periodic work.
4. ConfigManager persists key settings near executable.
