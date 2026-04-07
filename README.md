# JQB_WindowsLib

Native Windows GUI library for C++17 applications.

[![License: LGPL-3.0](https://img.shields.io/badge/License-LGPL--3.0-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0.html)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)]()
[![PlatformIO](https://img.shields.io/badge/PlatformIO-library-orange.svg)](https://platformio.org/)
[![Version](https://img.shields.io/badge/version-1.0.1-informational.svg)](library.json)

JQB_WindowsLib is a lightweight WinAPI-based application framework for building self-contained Windows desktop tools in modern C++.

It is designed for projects that want native controls, small deployment footprint, and a pragmatic API that feels closer to Arduino-style `setup()` / `loop()` than to heavyweight desktop frameworks.

## What It Is Good At

- internal engineering tools
- hardware utilities and configurators
- serial / BLE / HID desktop frontends
- dashboards and monitoring apps
- native helper apps distributed as a single `.exe`

## Core Value Proposition

JQB_WindowsLib gives you:

- a minimal application lifecycle: `init()`, `setup()`, `loop()`
- ready-made native UI widgets without a full framework dependency chain
- reusable I/O modules for Serial, BLE, HID, and Audio
- helper utilities for config files, dialogs, logging, timers, and hotkeys
- static-link friendly builds for simple Windows distribution

## Highlights

### Application Model

- Arduino-like global entry points
- automatic `WinMain` handling in `Core`
- simple component ownership through `SimpleWindow`

### UI Toolkit

- `SimpleWindow`, `OverlayWindow`, `LogWindow`, `TrayIcon`
- standard widgets: `Button`, `Label`, `InputField`, `Select`, `CheckBox`, `TextArea`
- visual widgets: `ProgressBar`, `Chart`, `ValueDisplay`, `ImageView`, `CanvasWindow`, `TabControl`
- utility UI helpers: `MenuBar`, `TreePanel`, `KeyCapture`

### Device I/O

- Serial COM support
- Bluetooth Low Energy scanning and GATT communication
- USB HID enumeration and feature report access
- audio input/output helpers with waveform generation

### Utilities

- UTF-8 / UTF-16 conversion helpers
- native folder/save/open dialogs
- INI-style config persistence
- debounce timer helpers
- CSV data logging
- global hotkey management
- header-only statistics aggregation

## Quick Start

### PlatformIO

Add the library to `platformio.ini`:

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

The library configures the important Win32 build details automatically via `scripts/compile_resources.py`, including:

- `-std=c++17`
- `-DUNICODE` and `-D_UNICODE`
- Windows subsystem build
- static runtime flags
- required Win32 link libraries
- resource compilation from `resources/resources.rc`

### Minimal Window Example

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>

SimpleWindow* window = nullptr;
Label* label = nullptr;

void setup() {
    window = new SimpleWindow(420, 220, "Hello JQB", 0);
    window->init();

    label = new Label(20, 20, 360, 28, L"Hello from WinAPI");
    window->add(label);

    window->add(new Button(20, 60, 140, 34, "Click me",
        [](Button*) {
            label->setText(L"Button clicked");
        }
    ));
}

void loop() {
}
```

## Example Application Patterns

### Serial Tool

Use `SimpleWindow`, `Select`, `TextArea`, and `IO/Serial` to build:

- firmware consoles
- sensor viewers
- protocol debug tools

### BLE Utility

Use `BLE`, `Chart`, `ValueDisplay`, and `LogWindow` to build:

- BLE scanners
- measurement dashboards
- live data viewers

### Custom Drawing App

Use `CanvasWindow` or `OverlayWindow` when you need:

- zoomable engineering views
- overlays for capture/streaming workflows
- GDI-based diagrams or path previews

## Main Modules

### Core

Files:

- `src/Core.h`
- `src/Core.cpp`

Responsibilities:

- `WinMain`
- message loop
- `_core` global object
- compatibility helpers for MinGW.org toolchains

### UI

Selected components:

- `SimpleWindow`: main application window and component host
- `OverlayWindow`: always-on-top standalone rendering surface
- `CanvasWindow`: zoomable / pannable drawing canvas
- `LogWindow`: detached log console window
- `TrayIcon`: taskbar notification icon with callbacks
- `MenuBar`: structured menu creation without raw boilerplate
- `TreePanel`: collapsible tree-like listbox UI helper

Standard controls:

- `Button`
- `Label`
- `InputField`
- `Select`
- `CheckBox`
- `TextArea`

Visual / advanced controls:

- `Chart`
- `ProgressBar`
- `ValueDisplay`
- `ImageView`
- `TabControl`
- `KeyCapture`

### IO

Modules:

- `IO/Serial`
- `IO/BLE`
- `IO/HID`
- `IO/Audio`

These modules dynamically load optional Windows APIs where needed, which keeps deployment simple and avoids turning the whole library into a dependency-heavy stack.

### Util

Helpers include:

- `StringUtils`
- `FileDialogs`
- `TimerUtils`
- `NumberUtils`
- `ConfigManager`
- `DataLogger`
- `HotkeyManager`
- `Statistics`

## Documentation Map

The repository already includes dedicated docs for most building blocks.

### Getting Started

- [Quick Start](docs/QuickStart.md)
- [Examples Overview](docs/examples/README.md)

### Core UI

- [Core](docs/Core.md)
- [SimpleWindow](docs/SimpleWindow.md)
- [OverlayWindow](docs/OverlayWindow.md)
- [CanvasWindow](docs/CanvasWindow.md)

### Controls

- [Button](docs/Button.md)
- [Label](docs/Label.md)
- [InputField](docs/InputField.md)
- [Select](docs/Select.md)
- [CheckBox](docs/CheckBox.md)
- [TextArea](docs/TextArea.md)
- [ProgressBar](docs/ProgressBar.md)
- [Chart](docs/Chart.md)
- [ValueDisplay](docs/ValueDisplay.md)
- [ImageView](docs/ImageView.md)
- [TabControl](docs/TabControl.md)

### IO

- [Serial](docs/Serial.md)
- [BLE](docs/BLE.md)
- [HID](docs/HID.md)
- [AudioEngine](docs/AudioEngine.md)
- [WaveGen](docs/WaveGen.md)

### Utilities

- [ConfigManager](docs/ConfigManager.md)
- [StringUtils](docs/StringUtils.md)
- [FileDialogs](docs/FileDialogs.md)
- [TimerUtils](docs/TimerUtils.md)

## Repository Layout

```text
JQB_WindowsLib/
├── library.json
├── LICENSE
├── README.md
├── docs/
├── scripts/
│   └── compile_resources.py
└── src/
    ├── Core.h / Core.cpp
    ├── IO/
    ├── UI/
    └── Util/
```

## Build and Toolchain Notes

### PlatformIO Native

The main target workflow is PlatformIO `platform = native` on Windows.

### MinGW Compatibility

The library is written with MinGW compatibility in mind, including older MinGW.org setups. For that reason it intentionally avoids some standard library features when WinAPI-based replacements are more reliable for the target toolchain.

Examples:

- `CreateThread` instead of `std::thread`
- explicit wide WinAPI calls such as `CreateWindowExW`
- `jqb_compat::to_wstring` instead of `std::to_wstring` where needed

### Self-Contained EXE Distribution

The build script applies static linking defaults so applications can typically be shipped as a single executable plus their own assets.

That makes the library especially suitable for device tools, factory utilities, field diagnostics, and other low-friction Windows deployments.

## FAQ

### Do I need to write raw WinAPI window procedures?

Usually no. `SimpleWindow`, the built-in controls, and helper classes remove most of the boilerplate. Raw WinAPI is still available when you need something specialized.

### Can I mix this with custom WinAPI controls?

Yes. The library is a thin wrapper around WinAPI concepts, not a sandbox. It is practical to combine library widgets with custom child windows, subclassing, or manual message handling.

### Is this meant to compete with Qt, wxWidgets, or .NET?

No. The goal is smaller, simpler, more native-feeling Windows tools with fewer moving parts and a lower abstraction penalty.

### Does it support multiple main windows?

`SimpleWindow` is intentionally singleton-like. For additional windows, use raw WinAPI or helper classes such as `OverlayWindow` / `LogWindow`.

## License

JQB_WindowsLib is licensed under the GNU Lesser General Public License, version 3 or later.

See [LICENSE](LICENSE).

If you distribute applications that statically link this library, make sure your distribution model also satisfies LGPL requirements for combined works, including license notice delivery and a practical relinking path for modified versions of the library.

## Maintainer

Created and maintained by [JAQUBA](https://github.com/JAQUBA).
