---
name: jqb-app-skeleton
description: "Generate a complete C++17 Windows app skeleton using JQB_WindowsLib with setup()/loop(), AppState/AppUI split, theming, logging, polling, and persistence-ready structure."
argument-hint: "Provide app name, required tabs/sections, and protocols to prepare"
user-invocable: true
---
# JQB App Skeleton

Create a production-ready starter for new desktop tools based on JQB_WindowsLib.

## Use When
- Starting a new app from scratch.
- Standardizing architecture across multiple products.
- Bootstrapping files for rapid feature iteration.

## Procedure
1. Confirm app purpose and required protocols (Serial/BLE/HID/Modbus/Audio).
2. Create core files from templates:
   - [main.cpp template](./assets/main.cpp.tpl)
   - [AppState.h template](./assets/AppState.h.tpl)
   - [AppState.cpp template](./assets/AppState.cpp.tpl)
   - [AppUI.h template](./assets/AppUI.h.tpl)
   - [AppUI.cpp template](./assets/AppUI.cpp.tpl)
3. Add base services in AppState:
   - ConfigManager
   - PollingManager
   - TextLogger
4. In UI initialization:
   - Create exactly one SimpleWindow
   - Call applyTheme(...)
   - Build first card section and primary action button
5. Wire loop() to lightweight periodic tick and polling.
6. Add docs seed with [README section template](./assets/README-section.md).

## Acceptance Criteria
- Builds with PlatformIO native environment.
- No std::thread usage.
- Wide WinAPI calls are explicit where relevant.
- setup()/loop() lifecycle remains minimal and clear.
