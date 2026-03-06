# JQB_WindowsLib

**C++ library for building native Windows desktop applications (WinAPI) with Arduino-like programming style.**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)]()
[![Build: PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange.svg)](https://platformio.org/)

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Project Structure](#project-structure)
- [UI Components](#ui-components)
- [IO Modules](#io-modules)
- [Utilities](#utilities)
- [Application Lifecycle](#application-lifecycle)
- [Complete Example](#complete-example)
- [API Reference](#api-reference)
- [FAQ](#faq)

---

## Overview

**JQB_WindowsLib** simplifies the creation of native Windows desktop applications with GUI. Inspired by the Arduino programming style (`setup()` / `loop()`), it hides WinAPI complexity behind a simple, object-oriented interface.

### Key Features

| Feature | Description |
|---------|-------------|
| **Arduino-like API** | `init()`, `setup()`, `loop()` functions — familiar programming model |
| **Ready-made UI components** | 13 components: windows, buttons, labels, charts, tabs, overlay... |
| **IO communication** | Serial (COM), Bluetooth Low Energy (BLE), USB HID |
| **Unicode / UTF-8** | Full support for international characters and Unicode |
| **Configuration management** | INI-style key=value settings save/load |
| **PlatformIO Native** | Build via PlatformIO (`platform = native`) |
| **Static linking** | Resulting `.exe` requires no additional DLLs |

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  Your Application                    │
│              init() → setup() → loop()               │
├─────────────────────────────────────────────────────┤
│                   JQB_WindowsLib                     │
│  ┌──────────┐  ┌──────────┐  ┌───────────────────┐  │
│  │   Core    │  │    UI    │  │        IO         │  │
│  │ WinMain  │  │ Window   │  │ Serial (COM)      │  │
│  │ MsgLoop  │  │ Overlay  │  │ BLE (Bluetooth)   │  │
│  │          │  │ Button   │  │ HID (USB HID)     │  │
│  │          │  │ Label    │  ├───────────────────┤  │
│  │          │  │ Select   │  │       Util        │  │
│  │          │  │ TextArea │  │ StringUtils       │  │
│  │          │  │ Chart    │  │ ConfigManager     │  │
│  │          │  │ CheckBox │  │ DataLogger        │  │
│  │          │  │ InputField│ │ HotkeyManager     │  │
│  │          │  │ ImageView│  │ Statistics        │  │
│  │          │  │ Progress │  │                   │  │
│  │          │  │ TabCtrl  │  │                   │  │
│  │          │  │ ValueDisp│  │                   │  │
│  └──────────┘  └──────────┘  └───────────────────┘  │
├─────────────────────────────────────────────────────┤
│                   Windows API (WinAPI)                │
│          GDI / GDI+ / COM / SetupAPI / BT            │
└─────────────────────────────────────────────────────┘
```

---

## Quick Start

### Minimal program — window with a button

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>

SimpleWindow* window;
Label* label;

void setup() {
    window = new SimpleWindow(400, 300, "My App", 0);
    window->init();

    label = new Label(20, 20, 360, 30, L"Hello World!");
    window->add(label);

    window->add(new Button(20, 60, 120, 35, "Click me",
        [](Button* btn) {
            label->setText(L"Button clicked!");
        }
    ));
}

void loop() {
    // Called cyclically in the message loop
}
```

### `platformio.ini` configuration

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

> **Note:** The library automatically adds the required flags: `-std=c++17`, `-DUNICODE`, `-D_UNICODE`, static linking, `subsystem:windows`, and the `gdi32`/`comctl32` libraries. You do not need to declare them manually.

---

## Installation

### Via PlatformIO (recommended)

Add to `platformio.ini`:

```ini
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

### Manual

1. Clone the repo:
   ```bash
   git clone https://github.com/JAQUBA/JQB_WindowsLib.git
   ```
2. Place in the `lib/` directory of your PlatformIO project or specify the path in `lib_deps`.

### Requirements

- **OS:** Windows 10+ (x64)
- **Compiler:** MinGW GCC (MinGW-w64 or MinGW.org) — provided by PlatformIO
- **C++ Standard:** C++17 (added automatically by the library)
- **PlatformIO Core:** 6.x+

---

## Project Structure

```
JQB_WindowsLib/
├── library.json              # PlatformIO manifest
├── scripts/
│   └── compile_resources.py  # Build auto-configuration + resource compilation
└── src/
    ├── Core.h / Core.cpp     # Core — WinMain, message loop
    ├── UI/                   # User interface components
    │   ├── UIComponent.h     #   Base interface (abstract)
    │   ├── SimpleWindow/     #   Main window (singleton)
    │   ├── OverlayWindow/    #   Overlay window (always-on-top, double-buffered GDI)
    │   ├── Button/           #   Button (click + long-press)
    │   ├── Label/            #   Text label (Unicode)
    │   ├── Select/           #   Dropdown list (ComboBox)
    │   ├── TextArea/         #   Multiline text field (readonly, font/color styling)
    │   ├── InputField/       #   Single-line edit field
    │   ├── CheckBox/         #   Checkbox
    │   ├── ProgressBar/      #   Progress bar (custom colors with visual styles bypass)
    │   ├── Chart/            #   Real-time chart
    │   ├── ValueDisplay/     #   Value display (LCD style)
    │   ├── ImageView/        #   Image display (BMP/PNG/JPG)
    │   └── TabControl/       #   Tab control
    ├── IO/                   # Communication
    │   ├── Serial/           #   Serial port (COM)
    │   ├── BLE/              #   Bluetooth Low Energy
    │   └── HID/              #   USB HID (Feature Reports)
    └── Util/                 # Utilities
        ├── StringUtils.*     #   UTF-8 ↔ UTF-16 conversions
        ├── ConfigManager.*   #   INI configuration save/load
        ├── DataLogger.*      #   CSV logger with auto-timestamp
        ├── HotkeyManager.*   #   Global keyboard shortcuts (WH_KEYBOARD_LL hook)
        └── Statistics.h      #   Header-only MIN/MAX/AVG/PEAK statistics
```

---

## UI Components

All components inherit from `UIComponent` and implement:
- `create(HWND parent)` — creates the native WinAPI control
- `getId()` — returns a unique identifier
- `getHandle()` — returns the HWND handle

### Component Summary

| Component | Description | Docs |
|-----------|-------------|------|
| **SimpleWindow** | Main application window (singleton) | [docs/SimpleWindow.md](docs/SimpleWindow.md) |
| **OverlayWindow** | Always-on-top overlay (subclassable, double-buffered GDI) | [docs/OverlayWindow.md](docs/OverlayWindow.md) |
| **Button** | Button with onClick and onLongClick | [docs/Button.md](docs/Button.md) |
| **Label** | Text label (Unicode, font/color styling) | [docs/Label.md](docs/Label.md) |
| **Select** | Dropdown list (ComboBox) | [docs/Select.md](docs/Select.md) |
| **TextArea** | Multiline text field (readonly, font/color styling) | [docs/TextArea.md](docs/TextArea.md) |
| **InputField** | Edit field with placeholder and validation | [docs/InputField.md](docs/InputField.md) |
| **CheckBox** | Checkbox with onChange callback | [docs/CheckBox.md](docs/CheckBox.md) |
| **ProgressBar** | Progress bar with Marquee mode and custom colors | [docs/ProgressBar.md](docs/ProgressBar.md) |
| **Chart** | Real-time measurement chart | [docs/Chart.md](docs/Chart.md) |
| **ValueDisplay** | LCD-style value display | [docs/ValueDisplay.md](docs/ValueDisplay.md) |
| **ImageView** | Images from file, resources, or memory | [docs/ImageView.md](docs/ImageView.md) |
| **TabControl** | Tab control with panels | [docs/TabControl.md](docs/TabControl.md) |

---

## IO Modules

| Module | Description | Docs |
|--------|-------------|------|
| **Serial** | COM port communication (RS-232) | [docs/Serial.md](docs/Serial.md) |
| **BLE** | Bluetooth Low Energy — scanning and communication | [docs/BLE.md](docs/BLE.md) |
| **HID** | USB HID — Feature Reports, device enumeration | [docs/HID.md](docs/HID.md) |

---

## Utilities

| Module | Description | Docs |
|--------|-------------|------|
| **StringUtils** | UTF-8 ↔ UTF-16/ANSI conversions | [docs/StringUtils.md](docs/StringUtils.md) |
| **ConfigManager** | Configuration manager (key=value format) | [docs/ConfigManager.md](docs/ConfigManager.md) |
| **DataLogger** | CSV logger with auto-timestamp filenames | [docs/DataLogger.md](docs/DataLogger.md) |
| **HotkeyManager** | Global keyboard shortcuts with settings dialog | [docs/HotkeyManager.md](docs/HotkeyManager.md) |
| **Statistics** | Header-only MIN/MAX/AVG/PEAK statistics | [docs/Statistics.md](docs/Statistics.md) |

---

## Application Lifecycle

```
Program Start
     │
     ▼
  Core() constructor   ← automatic, creates the global _core object
     │
     ▼
  init()              ← __weak — can be overridden; called in Core constructor
     │
     ▼
  WinMain()           ← Windows entry point
     │
     ▼
  setup()             ← create windows, components, connections
     │
     ▼
  ┌─── Message loop (GetMessage → TranslateMessage → DispatchMessage) ───┐
  │                                                                       │
  │   loop()  ← called after each message loop cycle                     │
  │                                                                       │
  └────────────────────────── WM_QUIT ends the loop ─────────────────────┘
     │
     ▼
  Program End
```

### Three callback functions (optional, `__weak`)

| Function | When | Typical use |
|----------|------|-------------|
| `init()` | Before `WinMain`, in `Core` constructor | Early initialization |
| `setup()` | After `WinMain`, before message loop | Create GUI, establish connections |
| `loop()` | In each message loop cycle | Update state, polling |

---

## Complete Example

### Application with window, buttons, Select, and Serial communication

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/Select/Select.h>
#include <UI/TextArea/TextArea.h>
#include <IO/Serial/Serial.h>

SimpleWindow* window;
Label*    lblStatus;
Select*   selPort;
TextArea* textLog;
Serial    serial;

void setup() {
    // Window initialization
    window = new SimpleWindow(600, 400, "Serial Monitor", 0);
    window->init();

    // Status
    lblStatus = new Label(20, 10, 560, 20, L"Disconnected");
    window->add(lblStatus);

    // COM port selection
    serial.init();
    selPort = new Select(20, 40, 200, 25, "Select port", [](Select* sel) {
        serial.setPort(sel->getText());
    });
    // Populate port list
    for (const auto& port : serial.getAvailablePorts()) {
        selPort->addItem(port.c_str());
    }
    window->add(selPort);

    // Connect button
    window->add(new Button(230, 40, 100, 25, "Connect", [](Button* btn) {
        if (serial.isConnected()) {
            serial.disconnect();
        } else {
            serial.connect();
        }
    }));

    // Communication log
    textLog = new TextArea(20, 80, 560, 280);
    window->add(textLog);

    // Data receive callback
    serial.onReceive([](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        textLog->append(text);
    });

    serial.onConnect([]() {
        lblStatus->setText(L"Connected!");
    });

    serial.onDisconnect([]() {
        lblStatus->setText(L"Disconnected");
    });
}

void loop() {
    // message loop handles events automatically
}
```

---

## API Reference

Full API documentation for each component is available in the [docs/](docs/) directory:

### Guides

- **[Quick Start](docs/QuickStart.md)** — get your first application running
- **[Examples](docs/examples/)** — 7 complete demo applications

### Components

- [Core](docs/Core.md) — application core
- [SimpleWindow](docs/SimpleWindow.md) — main window
- [OverlayWindow](docs/OverlayWindow.md) — overlay window
- [Button](docs/Button.md) — button
- [Label](docs/Label.md) — label
- [Select](docs/Select.md) — dropdown list
- [TextArea](docs/TextArea.md) — text field
- [InputField](docs/InputField.md) — edit field
- [CheckBox](docs/CheckBox.md) — checkbox
- [ProgressBar](docs/ProgressBar.md) — progress bar
- [Chart](docs/Chart.md) — chart
- [ValueDisplay](docs/ValueDisplay.md) — value display
- [ImageView](docs/ImageView.md) — image display
- [TabControl](docs/TabControl.md) — tabs

### IO & Util

- [Serial](docs/Serial.md) — COM communication
- [BLE](docs/BLE.md) — Bluetooth Low Energy
- [HID](docs/HID.md) — USB HID (Feature Reports)
- [StringUtils](docs/StringUtils.md) — string conversions
- [ConfigManager](docs/ConfigManager.md) — configuration manager
- [DataLogger](docs/DataLogger.md) — CSV data logger
- [HotkeyManager](docs/HotkeyManager.md) — keyboard shortcuts
- [Statistics](docs/Statistics.md) — MIN/MAX/AVG/PEAK statistics

---

## FAQ

### How do I add an icon to my application?

1. Create a `resources.rc` file in the project root:
   ```rc
   1 ICON "icon.ico"
   ```
2. Place the `icon.ico` file next to `resources.rc`.
3. The `compile_resources.py` script will automatically compile resources during build.
4. Use `SimpleWindow(width, height, "Title", 1)` — the last parameter is the icon resource ID.

### How do I handle Unicode / international characters?

The library fully supports Unicode. Use:
- `Label` accepts `const wchar_t*` — prefix `L`: `L"Ω µ °C"`
- `Button`, `Select`, and others accept `const char*` in UTF-8
- `StringUtils::utf8ToWide()` and `StringUtils::wideToUtf8()` for conversion

### Can I use it without PlatformIO?

Yes — the library is pure C++ with WinAPI. Requires linking: `gdi32`, `comctl32`. IO modules dynamically load: `setupapi`, `gdiplus`, `shlwapi`, `bthprops`, `hid`. Compilation: `g++ -std=c++17 -DUNICODE -D_UNICODE -mwindows -lgdi32 -lcomctl32 ...`

### How do I create a custom UI component?

Inherit from `UIComponent`:
```cpp
class MyWidget : public UIComponent {
public:
    void create(HWND parent) override { /* CreateWindowW(...) */ }
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }
private:
    HWND m_hwnd;
    int m_id;
};
```

---

## License

[GPL-3.0-or-later](https://www.gnu.org/licenses/gpl-3.0.html)

**Author:** [JAQUBA](https://github.com/JAQUBA)
