# JQB_WindowsLib — Copilot Instructions

## Project

A C++ library for building **native Windows desktop applications** with GUI.
Inspired by Arduino-style programming (`setup()` / `loop()`), it hides WinAPI complexity behind a simple, object-oriented interface.

### Purpose
Enable rapid creation of desktop tools (configurators, monitors, dashboards) for embedded devices communicating via Serial (COM), BLE, or HID.

---

## Architecture & Technology

| Parameter | Value |
|---|---|
| Language | **C++17** |
| Platform | **Windows 10+ (x64)** |
| GUI | **WinAPI** (native controls + custom GDI) |
| Build system | **PlatformIO** (`platform = native`) |
| Compiler | **MinGW GCC** (compatible with MinGW-w64 and MinGW.org) |
| Linking | Static (`-static -static-libgcc -static-libstdc++`) |
| Subsystem | Windows (`-Wl,-subsystem,windows`) |

### Linked Libraries

**Statically** (always): `gdi32`, `comctl32`, `comdlg32`, `winmm`

**Dynamically** (LoadLibrary/GetProcAddress in `init()`):

| DLL | Module | Functions |
|-----|--------|-----------|
| `hid.dll` | IO/HID | HidD_GetHidGuid, HidD_GetAttributes, HidD_Get/SetFeature, HidP_GetCaps |
| `setupapi.dll` | IO/HID, IO/Serial, IO/BLE | SetupDiGetClassDevs, SetupDiEnumDevice*, SetupDiGetDeviceInterfaceDetail, SetupDiGetDeviceRegistryProperty, SetupDiDestroyDeviceInfoList |
| `bthprops.cpl` | IO/BLE | BluetoothFindFirstRadio, BluetoothFindRadioClose, BluetoothGetRadioInfo |
| `BluetoothApis.dll` | IO/BLE | BluetoothGATTGetServices, BluetoothGATTGetCharacteristics, BluetoothGATTGetDescriptors, BluetoothGATTSetDescriptorValue, BluetoothGATTSetCharacteristicValue, BluetoothGATTRegisterEvent, BluetoothGATTUnregisterEvent |
| `gdiplus.dll` | UI/ImageView | GdiplusStartup/Shutdown, GdipCreateBitmapFrom*, GdipGetImage*, GdipCreateHBITMAPFromBitmap |
| `shlwapi.dll` | UI/ImageView | SHCreateMemStream |
| `uxtheme.dll` | UI/ProgressBar | SetWindowTheme (disables visual styles for custom colors) |

---

## Build Auto-Configuration (`compile_resources.py`)

The script `scripts/compile_resources.py` runs automatically via PlatformIO and adds:

| Flag | Description |
|------|-------------|
| `-DUNICODE -D_UNICODE` | Required by WinAPI (Wide versions) |
| `-std=c++17` | C++ standard required by the library |
| `-Wl,-subsystem,windows` | Build as Windows application (no console) |
| `-static-libgcc -static-libstdc++ -static` | Static linking |
| `-lgdi32 -lcomctl32 -lwinmm` | Required Windows libraries |
| Resource compilation `resources/resources.rc` | Icon and other Windows resources |
| `PROGNAME` from `VS_VERSION_INFO` | Auto-reads `InternalName` from `resources.rc` and sets output `.exe` name |

> **Do not declare these flags manually in `platformio.ini`** — they are added automatically.
> **Output filename:** If `resources/resources.rc` contains a `VS_VERSION_INFO` block with `InternalName`, the script automatically sets `PROGNAME` to that value (output binary = `InternalName.exe`). No per-project `extra_scripts` needed.

---

## MinGW.org Compatibility

The library is compatible with **MinGW.org GCC 6.3.0** (win32 thread model), which is the default PlatformIO `platform = native` compiler. Limitations:

| Missing in MinGW.org | Library replacement |
|---|---|
| `std::thread` | `CreateThread()` (WinAPI) |
| `std::to_wstring()` | `jqb_compat::to_wstring()` (defined in `Core.h`) |
| `swprintf_s()` | `_snwprintf()` |

> In application code, use `CreateThread()` instead of `std::thread` and `jqb_compat::to_wstring()` instead of `std::to_wstring()`.
> Always use explicit Wide WinAPI versions: `CreateFontW()`, `CreateWindowExW()`, `MessageBoxW()` etc.

---

## Programming Model (Arduino-like)

The application defines **three functions** (all optional, `__weak`):

```cpp
void init();    // Called in Core constructor (before WinMain)
void setup();   // Called once at the start of WinMain — create GUI
void loop();    // Called in each message loop cycle
```

### Global `_core` Object

```cpp
extern Core _core;
// _core.hInstance  — application HINSTANCE (needed for CreateWindow etc.)
// _core.nCmdShow   — window display mode
```

---

## Project Structure

```
src/
├── Core.h / .cpp          — WinMain, message loop, init/setup/loop, jqb_compat
scripts/
└── compile_resources.py   — build auto-configuration (flags, libs, resources)
├── UI/
│   ├── UIComponent.h      — abstract interface (create, getId, getHandle, handleClick, handleSelection, handleTextChange)
│   ├── SimpleWindow/       — main window (manages components, WindowProc) ⚠ singleton
│   ├── OverlayWindow/      — overlay window (raw WinAPI, always-on-top, double-buffered GDI, context menu, ConfigManager persistence)
│   ├── TrayIcon/           — system tray icon (Shell_NotifyIcon, context menu, callbacks)
│   ├── LogWindow/          — standalone log/console window (EDIT readonly, persistence, configurable colors)
│   ├── Button/             — button (onClick + onLongClick ≥ 800ms)
│   ├── Label/              — label (wchar_t*, Unicode, setFont/setTextColor/setBackColor)
│   ├── Select/             — ComboBox (onChange, link to vector)
│   ├── TextArea/           — edit multiline readonly (log/console, setFont/setTextColor/setBackColor)
│   ├── InputField/         — single-line edit (placeholder, password, maxLen, onTextChange fires on every keystroke)
│   ├── CheckBox/           — checkbox (onChange with bool)
│   ├── ProgressBar/        — progress bar (0-100%, Marquee, custom colors with theme bypass)
│   ├── Chart/              — real-time chart (GDI, auto-scale, time-window)
│   ├── ValueDisplay/       — LCD display (GDI, double-buffered, DisplayConfig)
│   ├── CanvasWindow/       — zoomable/pannable GDI canvas (child window, double-buffered, virtual onDraw, zoom/pan/grid)
│   ├── ImageView/          — images (GDI+, BMP/PNG/JPG, scale modes)
│   └── TabControl/         — tabs with panels
├── IO/
│   ├── Audio/              — Audio I/O (waveOut/waveIn, WaveGen, threaded double-buffering)
│   ├── Serial/             — COM port (threaded receive, auto-reconnect)
│   ├── BLE/                — Bluetooth LE (SetupAPI, GATT, overlapped I/O, scan/connect/notify/write)
│   ├── HID/                — USB HID (Feature Reports, device enumeration)
│   └── Modbus/             — Modbus RTU (ModbusSerialPort with full DCB control + RtuMaster FC01/02/03/04/05/06/15/16)
└── Util/
    ├── StringUtils.*       — UTF-8 ↔ UTF-16 ↔ ANSI, extractComPort
    ├── FileDialogs.*       — native folder/save dialogs with UTF-8 return paths
    ├── TimerUtils.*        — debounce timer helpers (`KillTimer`/`SetTimer` wrappers)
        ├── NumberUtils.*       — locale-tolerant number parsing helpers (comma/dot decimal)
    ├── ConfigManager.*     — key=value save/load (INI-like, auto-save)
    ├── DataLogger.*        — generic CSV logger with auto-timestamp
    ├── HotkeyManager.*     — global keyboard shortcuts (WH_KEYBOARD_LL hook, dialog, config)
    ├── PollingManager.*    — periodic polling groups (named, interval-driven, ticked from loop())
    ├── TextLogger.*        — INFO/ERROR/TX/RX log sink for TextArea (filterable, snapshot for export)
        ├── TreePanel/          — LISTBOX-based checkable tree widget with collapsible sections
    └── Statistics.h        — header-only MIN/MAX/AVG/PEAK statistics
```

---

## Component ID Ranges (auto-increment)

| Class | Starting ID |
|-------|-------------|
| Button | 1000 |
| Label | 2000 |
| InputField | 2000 |
| Select | 3000 |
| CheckBox | 3000 |
| TextArea | 4000 |
| ValueDisplay | 5000 |
| Chart | 5000 |
| ProgressBar | 6000 |
| TabControl | 7000 |
| ImageView | 8000 |

---

## UI Component Creation Patterns

### General Pattern

```cpp
UIComponent* component = new UIComponent(x, y, w, h, ...params...);
window->add(component);  // create() called automatically
```

### Window + Components

```cpp
SimpleWindow* window = new SimpleWindow(800, 600, "Title", iconResId);
window->init();

// Window background color (optional)
window->setBackgroundColor(RGB(30, 30, 38));

// Menu bar (optional)
HMENU menu = CreateMenu();
window->setMenu(menu);
window->onMenuCommand([](int cmdId) { /* routing */ });

// Components added via add() — lifetime managed by window
window->add(new Label(x, y, w, h, L"Text"));
window->add(new Button(x, y, w, h, "Text", [](Button* b) { ... }));
```

> `SimpleWindow` owns all components — destructor frees them.

### Do NOT manually delete components added to the window!

### Label — Font and Color Styling

```cpp
Label* lbl = new Label(x, y, w, h, L"Text");
window->add(lbl);  // FIRST add to window

// Font
lbl->setFont(L"Segoe UI", 14, true);           // fontName, size, bold
lbl->setFont(L"Consolas", 12, false, true);     // fontName, size, bold, italic

// Colors (handled automatically by WM_CTLCOLORSTATIC in SimpleWindow)
lbl->setTextColor(RGB(0, 220, 80));             // Text color
lbl->setBackColor(RGB(40, 40, 50));             // Background color
```

> Label colors require `WM_CTLCOLORSTATIC` handling — SimpleWindow does this automatically.

### TextArea — Font and Color Styling

```cpp
TextArea* log = new TextArea(x, y, w, h);
window->add(log);  // FIRST add to window

// Font
log->setFont(L"Consolas", 11, false);

// Colors (handled by WM_CTLCOLORSTATIC in SimpleWindow — same as Label)
log->setTextColor(RGB(170, 180, 195));
log->setBackColor(RGB(22, 22, 28));
```

> Readonly EDIT controls (TextArea) send `WM_CTLCOLORSTATIC`, same as STATIC labels.

### HotkeyManager — Shortcuts Dialog

```cpp
// Shortcut dialog — modal, managed by HotkeyManager
hotkeyMgr->showSettingsDialog(window->getHandle());
```

> Dialog automatically displays shortcut list, handles key combo capture, "Restore Defaults" button, and saves to INI file.

### OverlayWindow — Overlay Window (e.g. OBS)

```cpp
// Subclass with custom rendering
class MeterOverlay : public OverlayWindow {
public:
    MeterOverlay()
        : OverlayWindow(L"MyOverlay", L"My Overlay", 420, 160) {
        enablePersistence(config, "overlay"); // auto-save/load position and style
    }

    void updateData(const std::wstring& text) {
        m_text = text;
        invalidate(); // request repaint
    }

protected:
    void onPaint(HDC memDC, const RECT& rc) override {
        // Double buffering + background handled by base — only draw content
        SetTextColor(memDC, m_textColor);
        DrawTextW(memDC, m_text.c_str(), -1, (LPRECT)&rc,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

private:
    std::wstring m_text;
};

// Usage
auto* overlay = new MeterOverlay();
overlay->open();                          // WS_EX_TOOLWINDOW + WS_EX_TOPMOST
overlay->setBackgroundColor(RGB(0,0,0));  // black background
overlay->setTextColor(RGB(0,255,0));      // green text
overlay->setAlwaysOnTop(false);           // disable always-on-top
overlay->close();
```

> Right-click on overlay → built-in context menu (always-on-top, background/text chroma key colors, close).
> Subclass can extend the menu via `onBuildContextMenu(HMENU)` + `onMenuCommand(int)`.
> Base menu IDs: 9100–9149. Subclass IDs: 9150+.

### CanvasWindow — Zoomable/Pannable Canvas

```cpp
#include <UI/CanvasWindow/CanvasWindow.h>

// Subclass with custom rendering
class MyCanvas : public CanvasWindow {
protected:
    void onDraw(HDC hdc, const RECT& rc) override {
        // Use toScreenX() / toScreenY() for world→screen transforms
        MoveToEx(hdc, (int)toScreenX(0), (int)toScreenY(0), NULL);
        LineTo(hdc, (int)toScreenX(100), (int)toScreenY(50));
    }
};

// Usage
MyCanvas* canvas = new MyCanvas();
canvas->create(parentHwnd, 0, 60, 800, 540);
canvas->setGridVisible(true);
canvas->setGridSpacing(10.0);
canvas->setGridExtent(500.0, 500.0);
canvas->setBackgroundColor(RGB(30, 30, 38));
canvas->setGridColor(RGB(50, 50, 60));
canvas->redraw();
```

> Child window with own WndProc (`GWLP_USERDATA` pattern). Not a `UIComponent` — manage lifetime manually.
> Mouse: scroll = zoom towards cursor, left-drag = pan, double-click = reset view.
> World coordinates in mm; Y-axis flipped (positive Y = up).

### TrayIcon — System Tray Icon

```cpp
#include <UI/TrayIcon/TrayIcon.h>

TrayIcon* tray = new TrayIcon();
tray->create(hwnd, 101, L"My App");      // owner HWND, icon resource ID, tooltip
tray->setMenuLabels(L"Show", L"Exit");   // context menu labels (default: "Show"/"Exit")

tray->onRestore([]() {
    ShowWindow(hwnd, SW_SHOW);
    ShowWindow(hwnd, SW_RESTORE);
    SetForegroundWindow(hwnd);
    tray->hide();
});

tray->onExit([]() {                     // optional — default: PostMessage(WM_CLOSE)
    PostMessageW(hwnd, WM_CLOSE, 0, 0);
});

tray->show();                            // add icon to system tray
tray->hide();                            // remove from tray
tray->remove();                          // cleanup on exit
```

Integrate with `SimpleWindow` via `SetWindowSubclass()` to intercept `SC_MINIMIZE`:
```cpp
static LRESULT CALLBACK TrayProc(HWND hwnd, UINT msg, WPARAM wParam,
                                  LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (tray->processMessage(msg, wParam, lParam)) return 0;
    if (msg == WM_SYSCOMMAND && (wParam & 0xFFF0) == SC_MINIMIZE) {
        ShowWindow(hwnd, SW_HIDE);
        tray->show();
        return 0;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}
SetWindowSubclass(window->getHandle(), TrayProc, 1, 0);
```

> Command IDs: `TrayIcon::CMD_SHOW` (9200), `TrayIcon::CMD_EXIT` (9201).
> Message: `WM_TRAYICON` (`WM_APP + 100`).

### LogWindow — Standalone Log Window

```cpp
#include <UI/LogWindow/LogWindow.h>

LogWindow* logWin = new LogWindow();
logWin->setTitle(L"My App — Logs");       // window title
logWin->setFont(L"Consolas", 14);         // font (before open)
logWin->setTextColor(RGB(170, 180, 195)); // text color (before open)
logWin->setBackColor(RGB(22, 22, 28));    // background color (before open)
logWin->enablePersistence(config, "logwin"); // auto-save/load position

logWin->open(parentHwnd);                 // show log window
logWin->appendMessage(L"Hello!");         // add log line
logWin->clear();                          // clear all
logWin->close();                          // close and save position
logWin->isOpen();                         // check state
```

> Uses raw WinAPI `EDIT` control (readonly, multiline). Auto-scrolls on append.
> Position saved to ConfigManager as `prefix_x`, `prefix_y`, `prefix_w`, `prefix_h`.
> WndClass name: `JQB_LogWindow`.

---

## Text Encoding Convention

| Component | Parameter | Encoding |
|-----------|----------|----------|
| **Label** | text | `const wchar_t*` (prefix `L`) |
| **Button**, **Select**, **InputField**, **CheckBox**, **TabControl** | text | `const char*` (UTF-8) |
| **TextArea** | setText/append | Both: `char*` (UTF-8) and `wchar_t*` (Unicode) |
| **ValueDisplay** | updateValue, setMode | `std::wstring` |

Conversions: `StringUtils::utf8ToWide()` / `StringUtils::wideToUtf8()`

---

## Callback Patterns

### Button — Click and Long Press

```cpp
new Button(x, y, w, h, "Text",
    [](Button* btn) { /* click */ },
    [](Button* btn) { /* long press ≥ 800ms */ }
);
```

### Select — Selection Change

```cpp
new Select(x, y, w, h, "Default",
    [](Select* sel) {
        const char* text = sel->getText();  // selected text
    }
);
```

### CheckBox — State Change

```cpp
new CheckBox(x, y, w, h, "Option", false,
    [](CheckBox* cb, bool checked) { /* ... */ }
);
```

### InputField — Text Change (fires on every keystroke via EN_CHANGE)

```cpp
new InputField(x, y, w, h, "",
    [](InputField* field, const char* text) { /* fires on every keystroke */ }
);
```

### Serial — Events

```cpp
serial.onConnect([]() { /* ... */ });
serial.onDisconnect([]() { /* ... */ });
serial.onReceive([](const std::vector<uint8_t>& data) { /* ... */ });
```

### BLE — Events

```cpp
ble.onConnect([]() { /* ... */ });
ble.onDisconnect([]() { /* ... */ });
ble.onReceive([](const std::vector<uint8_t>& data) { /* ... */ });
ble.onDeviceDiscovered([](const BLEDevice& dev) { /* ... */ });
ble.onScanComplete([]() { /* ... */ });
ble.onError([](const std::wstring& msg) { /* ... */ });
```

### BLE — Filtering and Prioritization

```cpp
// Scan with device name filter
ble.startScan(BLEScanFilter(L"MyDevice"), 10);

// Prioritization — matching devices appear first in the list
ble.addPriorityFilter(L"owon");
ble.addPriorityFilter(L"ow18b");
ble.startScan(10);
```

---

## Device Communication

### Serial (COM)

```cpp
Serial serial;
serial.init();                       // Scans ports
serial.setPort("COM3");              // Set port
serial.onReceive([](auto& data) {  // Receive callback
    // data = std::vector<uint8_t>
});
serial.connect();                    // Opens port + read thread
serial.write({0x55, 0xAA, 0x01});   // Send
serial.disconnect();                 // Close
```

### BLE (Bluetooth Low Energy)

```cpp
BLE ble;
ble.init();                          // Check BT adapter, load BluetoothApis.dll

// GATT configuration (service and characteristic UUIDs)
ble.setServiceUUID(L"0000fff0-0000-1000-8000-00805f9b34fb");
ble.setNotifyCharacteristicUUID(L"0000fff4-0000-1000-8000-00805f9b34fb");
ble.setWriteCharacteristicUUID(L"0000fff3-0000-1000-8000-00805f9b34fb");

ble.startScan(10);                   // Scan for 10 seconds
ble.onScanComplete([&]() {
    auto& devices = ble.getDiscoveredDevices();
    if (!devices.empty()) {
        ble.connect(devices[0].address);  // Automatic GATT setup
    }
});
ble.onReceive([](auto& data) { /* Data from GATT notify */ });
```

### HID (USB Human Interface Device)

```cpp
HID hid;
hid.init();                          // Load hid.dll dynamically
hid.setVidPid(0x1209, 0xC55D);       // Device VID/PID
hid.setUsage(0xFF00, 0x01);          // Usage Page + Usage
hid.setFeatureReportSize(7);         // Feature Report data size
hid.findAndOpen();                   // Find and open device

uint8_t buf[7];
hid.getFeatureReport(2, buf, 7);     // Read Feature Report
hid.setFeatureReport(2, buf, 7);     // Write Feature Report
hid.close();                         // Close
```

### Audio (waveOut / waveIn)

```cpp
#include <IO/Audio/AudioEngine.h>

AudioEngine engine;

// Enumerate devices
auto outputs = engine.enumOutputDevices();
auto inputs  = engine.enumInputDevices();

// Configure waveform generator
engine.getWaveGen().setWaveform(WAVE_SINE);
engine.getWaveGen().setFrequency(440.0);
engine.getWaveGen().setAmplitude(0.5);

// Start output / input
engine.startOutput(0);          // device index
engine.startInput(0);

// In loop() — read thread-safe snapshots
double buf[256];
int count;
if (engine.getOutputSnapshot(buf, 256, count)) {
    for (int i = 0; i < count; i++)
        chart->addDataPoint(buf[i], L"");
}
if (engine.getInputSnapshot(buf, 256, count)) {
    // process input data...
}

engine.stopOutput();
engine.stopInput();
```

> Requires `winmm` — linked automatically by `compile_resources.py`.

---

## Application Configuration

```cpp
ConfigManager config("settings.ini");

// Write
config.setValue("port", "COM3");

// Read (with default value)
std::string port = config.getValue("port", "COM1");

// Auto-save on destructor
```

File format:
```ini
# Comment
port=COM3
baudrate=9600
```

---

## Copilot Guidelines

1. **Always use C++17** — standard added automatically, do not declare manually
2. **Three main functions:** `init()`, `setup()`, `loop()` — defined globally, not in a class
3. **Label takes `wchar_t*`** with `L` prefix — other components use UTF-8 `char*`
4. **Label.setFont/setTextColor/setBackColor** — font and color styling (colors handled by SimpleWindow WM_CTLCOLORSTATIC)
5. **TextArea.setFont/setTextColor/setBackColor** — same styling API as Label, also uses WM_CTLCOLORSTATIC
6. **SimpleWindow.setBackgroundColor()** — changes window background (WM_ERASEBKGND)
7. **SimpleWindow.setMenu(HMENU)** + **onMenuCommand(callback)** — menu bar support
8. **SimpleWindow is a singleton** (`s_instance`) — do not create a second one! For additional windows use `OverlayWindow` or raw WinAPI with `GWLP_USERDATA`
9. **HotkeyManager.showSettingsDialog(HWND)** — built-in keyboard shortcuts dialog
10. **Do not delete components added to SimpleWindow** — window manages their memory
11. **Include `<Core.h>`** always as the first include
12. **Callbacks:** Use lambdas `[capture](Type* ptr) { ... }`
13. **New window:** `new SimpleWindow(w, h, "title", 0); window->init();` — ALWAYS call `init()`!
14. **Serial/BLE/HID:** Always `init()` before `connect()` / `startScan()` / `findAndOpen()` — init() loads DLLs dynamically
15. **String conversion:** `StringUtils::utf8ToWide()` / `wideToUtf8()`
16. **Static linking:** Resulting `.exe` needs no extra DLLs (system dependencies loaded dynamically)
17. **Resources (icon):** `resources/resources.rc`, compiled automatically
18. **TextArea is readonly** — for editable fields use `InputField`
19. **Chart** automatically removes old data and limits FPS (`setTimeWindow(double)`, `setRefreshRate()`). Supports `setTriggerEnabled(true)` for oscilloscope-style rising zero-crossing sync (backward search, 3× data retention). `setLineWidth(int)` controls data line width (default 2).
20. **ValueDisplay** supports double-buffering, `DisplayConfig` (colors, fonts, proportions), custom `ValueFormatter`
21. **TabControl::getTabPage()** returns panel HWND — place child controls on it
22. **Dynamic DLL loading:** IO modules (Serial, BLE, HID) and ImageView load their DLLs via `LoadLibrary`/`GetProcAddress` in `init()`. Only `gdi32` and `comctl32` are statically linked.
23. **MinGW.org compat:** Do not use `std::thread` (use `CreateThread`), `std::to_wstring` (use `jqb_compat::to_wstring`), `swprintf_s` (use `_snwprintf`).
24. **WinAPI Wide:** Always use explicit `W` versions: `CreateFontW()`, `CreateWindowExW()`, `MessageBoxW()`.
25. **Statistics** — header-only (`Util/Statistics.h`), `addSample()` / `reset()`, fields `min/max/peak/sum/count`
26. **DataLogger** — CSV with auto-timestamp filename, `startRecording(headers)` / `addRow(columns)` / `stopRecording()`
27. **HotkeyManager** — global `WH_KEYBOARD_LL` hook, `addHotkey(iniKey, label, default, action)`, `loadFromConfig()`, `installHook()`, built-in `showSettingsDialog(HWND)`
28. **ConfigManager** — INI key=value, auto-save on destructor, `setValue()` / `getValue(key, default)`
29. **LoadCursorW with IDC_ARROW** — in MinGW requires cast: `LoadCursorW(NULL, (LPCWSTR)IDC_ARROW)`
30. **Control IDs 1000-8999** (auto), **Menu IDs 9000+** (manual), **Context menu IDs 9100+** — prevents collisions. **Note:** Label and InputField both start at 2000 — SimpleWindow routes `EN_CHANGE` by HWND (not ID) to avoid collisions
31. **OverlayWindow** — base overlay window class with `virtual onPaint()`, double-buffered GDI, always-on-top, context menu (chroma key colors), `enablePersistence(config, prefix)` for auto-save/load position and style
32. **ProgressBar custom colors** — `setColor()` / `setBackColor()` automatically disable visual styles on the control (via `uxtheme.dll` → `SetWindowTheme`) to ensure `PBM_SETBARCOLOR` works with Common Controls v6
33. **TrayIcon** — system tray icon (`UI/TrayIcon/TrayIcon.h`), `create()` / `show()` / `hide()` / `remove()`, configurable menu labels via `setMenuLabels()`, `onRestore()` + `onExit()` callbacks, integrate with `SimpleWindow` via `SetWindowSubclass()` + `processMessage()`. IDs: 9200-9201. Message: `WM_TRAYICON` (`WM_APP + 100`).
39. **NumberUtils** — `<Util/NumberUtils.h>`, `NumberUtils::parseDouble(const std::string&)` (throws on invalid input), `NumberUtils::tryParseDouble(const std::string&, double&)` (non-throwing), `NumberUtils::formatDouble(double, int decimals)`. Accepts both `'.'` and `','` as decimal separator. Use instead of inline `std::stod` with manual comma→dot replacement.
40. **TreePanel** — `<UI/TreePanel/TreePanel.h>`, LISTBOX-based collapsible tree widget with checkboxes. Construct with `TreePanel(HWND hListbox)`. Build tree with `clear()` + `addSection()` / `addExpandGroup()` / `addItem()` / `addActionItem()`. Handle WM_COMMAND/LBN_SELCHANGE via `handleClick(idx)` — returns `true` if UI state changed (caller should call clear() + add*() to rebuild). `addItem()` accepts optional `onToggle` callback (fired after flag toggle, useful for cache invalidation side-effects). `addActionItem()` fires a callback on click. `getClickedFlag(idx)` returns the `bool*` of the clicked item (for additional app-level reactions). Indent levels: 0=section header, 1=item (2 sp), 2=sub-item (6 sp), 3=sub-sub-item (10 sp).
34. **LogWindow** — standalone log window (`UI/LogWindow/LogWindow.h`), `open()` / `close()` / `appendMessage()` / `clear()`, configurable font/colors via `setFont()` / `setTextColor()` / `setBackColor()` (call before `open()`), `enablePersistence(config, prefix)` for position auto-save. Not a `UIComponent` — standalone WinAPI window with `GWLP_USERDATA` pattern.
35. **AudioEngine** — `<IO/Audio/AudioEngine.h>`, `startOutput(deviceIndex)` / `startInput(deviceIndex)` / `stopOutput()` / `stopInput()`, thread-safe snapshots via `getOutputSnapshot()` / `getInputSnapshot()` (protected by `CRITICAL_SECTION`). Triple-buffering (`AUDIO_NUM_BUFFERS=3`). Auto sample rate negotiation: `setSampleRate(preferred)` + `startOutput()`/`startInput()` try 192k→96k→48k→44.1k. `getActualSampleRate()` returns the negotiated rate. `winmm` linked automatically by `compile_resources.py`.
36. **WaveGen** — `engine.getWaveGen()` returns `WaveGen&`. `setWaveform()`, `setFrequency()`, `setAmplitude()`, `resetPhase()`. Enum: `WAVE_SINE`, `WAVE_SAWTOOTH`, `WAVE_TRIANGLE`, `WAVE_SQUARE`, `WAVE_WHITE_NOISE`.
37. **Audio constants** — `AUDIO_SAMPLE_RATE` (48000 default/fallback), `AUDIO_BUFFER_SAMPLES` (4096), `AUDIO_NUM_BUFFERS` (3), `AUDIO_DOWNSAMPLE` (8 default), `AUDIO_SNAPSHOT_SIZE` (`AUDIO_BUFFER_SAMPLES`). Configurable downsample via `engine.setDownsampleFactor()`. Configurable sample rate via `engine.setSampleRate()` (auto-negotiated on start).
38. **CanvasWindow** — reusable zoomable/pannable GDI canvas (`UI/CanvasWindow/CanvasWindow.h`). `create(parent, x, y, w, h)`, `redraw()`, `resetView()`, `setGridVisible()` / `setGridSpacing()` / `setGridExtent()` / `setGridColor()`, `setBackgroundColor()`, `setDefaultZoom()` / `setDefaultPan()`. Subclass overrides `virtual onDraw(HDC, RECT)`. Coordinate helpers: `toScreenX(worldX)` / `toScreenY(worldY)`. World units in mm, Y-axis flipped. Mouse: wheel = zoom towards cursor, left-drag = pan, double-click = reset. Child window with `GWLP_USERDATA` pattern (not a `UIComponent`).
41. **Theme** — `<UI/Theme/Theme.h>`, runtime `struct Theme { bg, surface, surface2, text, textDim, accent, accentHover, ok, warn, err, fontName, fontSize }`. Static palette factories: `Theme::catppuccinMocha/Frappe/Latte()`, `nord()`, `dracula()`, `tokyoNight()`, `oneDark()`, `gruvboxDark()`, `light()`. Free functions: `applyTheme(SimpleWindow*, const Theme&)` (sets bg + default text color + auto-styles all buttons + recolors all `ProgressBar`s + themes all `TabControl` pages + enables Win10/11 immersive dark title bar via `DwmSetWindowAttribute(20, ...)` for dark palettes), `styleAccentButton(Button*, const Theme&)`, `styleSecondaryButton(Button*, const Theme&)`. Coexists with the older macro-based palette headers (`Theme/CatppuccinMocha.h` etc.) — both APIs are valid. **For UI layout best practices, see [docs/UIDesignGuide.md](../docs/UIDesignGuide.md).**
42. **PollingManager** — `<Util/PollingManager.h>`, top-level `class PollingManager` with nested `Group{name, intervalMs, enabled, nextTick, action}`. `addGroup(name, ms, action)` / `setEnabled(name, bool)` / `setInterval(name, ms)` / `trigger(name)` (run once now) / `tick()` (call from `loop()`). Single-threaded — actions run on UI thread.
43. **TextLogger** — `<Util/TextLogger.h>`, generic `TextArea` sink. `attach(TextArea*)`, `info(wstring)`, `error(wstring)`, `tx(tag, bytes)`, `rx(tag, bytes)` (auto hex-format), `snapshot()` (full buffer for export), `clear()`. Live filter flags: `showInfo / showError / showTx / showRx`. No protocol coupling.
44. **Modbus RTU stack** — `<IO/Modbus/ModbusSerialPort.h>` + `<IO/Modbus/ModbusRTU.h>`, in `modbus::` namespace. Use `ModbusSerialPort` (NOT the general-purpose `Serial`) when you need DCB control: parity, stop bits, data bits, hard read timeout. `RtuMaster(port)` provides FC01/02/03/04/05/06/15/16, returns `Result{status, exceptionCode, message}`. Helpers: `crc16(data, len)` (poly 0xA001), `toHex(bytes)`. Synchronous — call from worker thread (`CreateThread`) for long scans, not from `loop()`. `ModbusSerialPort::enumPorts()` returns `std::vector<std::string>` of available COM ports.
45. **SimpleWindow accessors (read-only, for theming)** — `getComponents()`, `getButtons()`, `getLabels()`, `getTextAreas()`, `getBackgroundColor()`, `getDefaultTextColor()`. Used by `applyTheme()`. Backwards-compatible additions; safe to call from your own code.
46. **TabControl page theming** — `setPageBackground(COLORREF)` recolors all tab pages with a custom brush (replaces the default white STATIC fill). Auto-called by `applyTheme()`. Pages are now plain `STATIC` (no `SS_WHITERECT`) and the page subclass paints `WM_ERASEBKGND` via a brush stored on each page (`SetPropW(L"JQB_TabPageBrush")`).
47. **UI Design Guide** — [docs/UIDesignGuide.md](../docs/UIDesignGuide.md). Prescriptive rules for laying out apps: cards, section headers, field labels, accent buttons, status footer, 8/16/24 spacing rule, threading, polling, anti-patterns. **Read before laying out a new app.** Reference apps: WektoroweLitery2, gerber2gcode, Konfigurator.

### Typical Application Layout

```
┌──────────────────────────────────────┐
│ Window Title                   [_][□][X] │
├──────────────────────────────────────┤
│ Label: "Status"                      │
│ [Select ▼ COM3] [Connect] [Disconnect] │
│ ┌──────────────────────────────────┐ │
│ │ TextArea (log/console)           │ │
│ │ > Received: 55 AA 01 ...        │ │
│ │ > Sent: 55 AA 02 ...            │ │
│ └──────────────────────────────────┘ │
│ [Send] [Clear]                       │
└──────────────────────────────────────┘
```

### Minimal `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

> C++17, UNICODE, static linking, and library flags are added automatically by `compile_resources.py`.
> Output binary name is set automatically from `InternalName` in `resources/resources.rc` `VS_VERSION_INFO`.

### Minimal `main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(400, 300, "Hello JQB", 0);
    window->init();
    window->add(new Label(20, 20, 360, 30, L"Hello!"));
}

void loop() {}
```

---

## Architecture Limitations

### SimpleWindow — Singleton

`SimpleWindow` uses a static field `s_instance` for `WindowProc` routing — only **one main window** per process. Creating a second `SimpleWindow` will overwrite the pointer and break the first window.

**Additional windows** should be created via raw WinAPI (`GWLP_USERDATA` pattern) or `OverlayWindow` subclass:

```cpp
class MyOverlay : public OverlayWindow {
public:
    MyOverlay()
        : OverlayWindow(L"MyOverlay", L"My Overlay", 420, 160) {}

protected:
    void onPaint(HDC memDC, const RECT& rc) override {
        // Custom GDI rendering
    }
};
```

### Modal Dialogs

Pattern for blocking the main window:
```cpp
EnableWindow(parentHwnd, FALSE);   // Block parent
// ... CreateWindowExW with WS_EX_DLGMODALFRAME ...
// In WM_DESTROY:
EnableWindow(parentHwnd, TRUE);    // Unblock parent
```

See: `HotkeyManager::showSettingsDialog()` — keyboard shortcuts dialog.

---

## Util Tools — Full Documentation

### Statistics (header-only)

```cpp
#include <Util/Statistics.h>

Statistics stats;
stats.addSample(3.14);
stats.addSample(-1.5);
// stats.min = -1.5, stats.max = 3.14, stats.getAvg() = 0.82, stats.peak = 3.14
stats.reset();
```

Ignores `NaN` and `Inf`. Field `hasData` indicates whether samples were added. Field `count` → sample count.

### DataLogger (CSV)

```cpp
#include <Util/DataLogger.h>

DataLogger logger("measurements");  // File prefix
logger.startRecording({"Value", "Unit", "Mode"});  // CSV headers
logger.addRow({"3.14", "V", "DC V"});               // Data row
logger.getSampleCount();                             // Row count
logger.getElapsedSeconds();                          // Recording duration
logger.stopRecording();
// File: measurements_2026-03-05_14-30-45.csv
```

Auto-timestamp in filename. `isRecording()` to check state.

### HotkeyManager (Keyboard Shortcuts)

```cpp
#include <Util/HotkeyManager.h>
#include <Util/ConfigManager.h>

ConfigManager config("app.ini");
HotkeyManager* hkMgr = new HotkeyManager(config);

// Register shortcuts (iniKey, label, defaultBind, action)
hkMgr->addHotkey("shortcut_save", "Save", "Ctrl+S", []() { doSave(); });
hkMgr->addHotkey("shortcut_quit", "Quit", "Ctrl+Q", []() { doQuit(); });

hkMgr->loadFromConfig();   // Load bindings from INI
hkMgr->installHook();      // Install global WH_KEYBOARD_LL hook
// ...
hkMgr->showSettingsDialog(parentHwnd);  // Modal shortcut editing dialog
```

Dialog automatically handles: shortcut list, key combo capture, restore defaults, save to INI.

### FileDialogs (Folder / Save Dialogs)

```cpp
#include <Util/FileDialogs.h>

std::string dir = FileDialogs::browseFolderUTF8(hwnd, L"Select folder");
std::string out = FileDialogs::saveFileDialogUTF8(
    hwnd,
    L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0",
    L"Export",
    L"txt");
```

Both functions return UTF-8 paths and empty string on cancel.

### TimerUtils (Debounce Timers)

```cpp
#include <Util/TimerUtils.h>

TimerUtils::restartDebounceTimer(hwnd, 9601, 400);
// ... on WM_TIMER:
TimerUtils::stopTimer(hwnd, 9601);
```

Use for debounced UI reactions after text edits or rapid option changes.

---

## BLE Sensor Application Pattern

A typical BLE sensor application uses a data pipeline:

```
BLE.onReceive() → parse() → ValueDisplay + Chart + Statistics + DataLogger + Log
```

### Pipeline Fan-out (Multi-consumer)

```cpp
void handleSensorData(const std::vector<uint8_t>& data) {
    // 1. Parse (device protocol specific)
    auto measurement = MyParser::parse(data);
    if (!measurement.valid) return;

    // 2. LCD display
    valueDisplay->updateValue(measurement.value, measurement.prefix, measurement.unit);
    valueDisplay->setMode(measurement.modeStr);

    // 3. Real-time chart
    if (chartEnabled)
        chart->addDataPoint(measurement.value, measurement.unit);

    // 4. Statistics
    stats.addSample(measurement.value);
    updateStatsLabel();

    // 5. CSV recording
    if (dataLogger.isRecording())
        dataLogger.addRow({valueStr, unitStr, modeStr});

    // 6. Text log
    logMsg(formatMessage(measurement));
}
```

### Menu — ID Range Pattern

UI controls have auto-IDs in range 1000-8999. Define menu commands in range **9000+** to avoid collisions:

```cpp
// MenuHandler.h
#define IDM_FILE_SAVE      9001
#define IDM_FILE_EXIT      9002
#define IDM_CONN_SCAN      9010
#define IDM_CONN_CONNECT   9011
// Overlay/separate window context menu IDs: 9100+
#define IDM_OVL_CLOSE      9100
```

Routing via `switch(cmdId)` in `window->onMenuCommand(...)` callback.
