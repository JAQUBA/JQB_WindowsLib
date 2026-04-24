# Quick Start — JQB_WindowsLib

A quick guide to launching your first application.

---

## 1. Requirements

- [PlatformIO](https://platformio.org/) (as VS Code extension or CLI)
- **MinGW-w64 (GCC)** compiler — PlatformIO downloads it automatically
- Windows 10+ (x64)

---

## 2. Creating a Project

### `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

> C++17, UNICODE, static linking, and library flags are added automatically by the library.

### `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(500, 350, "My Application", 0);
    window->init();

    window->add(new Label(20, 20, 460, 30, L"Welcome to JQB_WindowsLib!"));

    window->add(new Button(20, 70, 150, 35, "Click me", [](Button* btn) {
        MessageBoxW(NULL, L"Button clicked!", L"Info", MB_OK);
    }));
}

void loop() {
    // Called in each message loop cycle — continuous logic goes here
}
```

### 3. Building and Running

```bash
pio run                # Compile
pio run --target exec  # Run (if configured)
```

The resulting `.exe` can be found in `.pio/build/app/program.exe`.

---

## 4. Adding an Icon (optional)

1. Create a `resources/` folder in the project root.
2. Place an `app.ico` file in the `resources/` folder.
3. Create `resources/resources.rc`:
   ```rc
   101 ICON "app.ico"
   ```
3. In `SimpleWindow` pass the icon ID:
   ```cpp
   window = new SimpleWindow(800, 600, "Title", 101);
   ```

The `compile_resources.py` script from the library automatically compiles `.rc` to `.res`.

> **Output filename:** If `resources.rc` contains a `VS_VERSION_INFO` block with `InternalName`, the script automatically sets the output binary name to that value (e.g. `MyApp.exe`).

---

## 5. Application Lifecycle

```
Program start
    │
    ▼
  init()          ← optional: global variables, early configuration
    │
    ▼
  setup()         ← create windows, components, initialize I/O
    │
    ▼
  ┌─── loop() ◄──┐
  │               │  ← Windows message loop + loop()
  └───────────────┘
    │
    ▼
  Program end
```

- `init()` — called in `Core` constructor (before `WinMain`)
- `setup()` — called once at the start of `WinMain`
- `loop()` — called in each message loop cycle

All three functions are `__weak` — define only the ones you need.

---

## 6. Common Patterns

### Updating a Label from loop()

```cpp
Label* lblTime;

void setup() {
    window = new SimpleWindow(400, 200, "Clock", 0);
    window->init();
    lblTime = new Label(20, 20, 360, 30, L"00:00:00");
    window->add(lblTime);
}

void loop() {
    static DWORD lastUpdate = 0;
    DWORD now = GetTickCount();
    if (now - lastUpdate >= 1000) {
        lastUpdate = now;
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t buf[32];
        swprintf(buf, 32, L"%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);
        lblTime->setText(buf);
    }
}
```

### Serial Communication

```cpp
#include <IO/Serial/Serial.h>
Serial serial;

void setup() {
    // ... create window ...
    serial.init();
    serial.setPort("COM3");
    serial.onReceive([](const std::vector<uint8_t>& data) {
        // process received data
    });
    serial.connect();
}
```

### Saving Configuration

```cpp
#include <Util/ConfigManager.h>
ConfigManager config("settings.ini");

void setup() {
    std::string port = config.getValue("port", "COM1");
    // ... use port ...
}

// config auto-saves on close (destructor)
```

---

## 7. Next Steps

- [Documentation Hub](README.md) — central index for all docs
- [Component Documentation](.) — details of each component
- [Examples](examples/) — ready-to-use demo applications
- [Runnable Example Projects](../examples/README.md) — complete project folders you can build directly
- [Project Blueprint](ProjectBlueprint.md) — production app structure
- [AI Assistants Guide](AI_Assistants.md) — prompt templates for Copilot and Claude Code
- [Copilot Instructions](../.github/copilot-instructions.md) — repository rules for GitHub Copilot
- [Claude Instructions](../CLAUDE.md) — repository rules for Claude Code
