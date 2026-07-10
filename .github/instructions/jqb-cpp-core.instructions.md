---
name: JQB Cpp Core Rules
description: "Use when editing C++ sources in JQB_WindowsLib or apps based on it. Enforces C++17, MinGW-safe patterns, wide WinAPI calls, and window/component ownership rules."
applyTo:
  - "src/**/*.h"
  - "src/**/*.cpp"
  - "examples/**/*.h"
  - "examples/**/*.cpp"
---
# JQB C++ Core Rules

- Use C++17-compatible constructs.
- Prefer explicit wide WinAPI forms (CreateWindowExW, MessageBoxW, CreateFontW, etc.).
- Do not use std::thread; use CreateThread when background tasks are needed.
- Do not create a second SimpleWindow main instance.
- Components passed to window->add(...) are owned by SimpleWindow and should not be manually deleted.
- Keep setup()/loop() entry points lightweight in applications.
- Keep protocol-heavy logic outside direct UI control callbacks when possible.
