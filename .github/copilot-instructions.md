# JQB_WindowsLib — Copilot Instructions

## Projekt

Biblioteka C++ do tworzenia **natywnych aplikacji desktopowych Windows** z GUI.  
Inspirowana stylem Arduino (`setup()` / `loop()`), ukrywa złożoność WinAPI za prostym, obiektowym interfejsem.

### Cel biblioteki
Umożliwienie szybkiego tworzenia narzędzi desktopowych (konfiguratory, monitory, dashboardy) dla urządzeń embedded komunikujących się przez Serial (COM), BLE lub HID.

---

## Architektura i technologie

| Parametr | Wartość |
|---|---|
| Język | **C++17** |
| Platforma | **Windows 10+ (x64)** |
| GUI | **WinAPI** (natywne kontrolki Windows + custom GDI) |
| Build system | **PlatformIO** (`platform = native`) |
| Kompilator | **MinGW-w64 (GCC)** |
| Linkowanie | Statyczne (`-static -static-libgcc -static-libstdc++`) |
| Subsystem | Windows (`-Wl,-subsystem,windows`) |

### Linkowane biblioteki

`gdi32`, `comctl32`, `setupapi`, `gdiplus`, `shlwapi`, `bthprops`, `bluetoothapis`, `ole32`, `uuid`

---

## Model programowania (Arduino-like)

Aplikacja definiuje **trzy funkcje** (wszystkie opcjonalne, `__weak`):

```cpp
void init();    // Wywoływana w konstruktorze Core (przed WinMain)
void setup();   // Wywoływana raz na początku WinMain — tworzenie GUI
void loop();    // Wywoływana w każdym cyklu pętli komunikatów Windows
```

### Globalny obiekt `_core`

```cpp
extern Core _core;
// _core.hInstance  — HINSTANCE aplikacji (potrzebne do CreateWindow itp.)
// _core.nCmdShow   — tryb wyświetlania okna
```

---

## Struktura projektu

```
src/
├── Core.h / .cpp          — WinMain, message loop, init/setup/loop
├── UI/
│   ├── UIComponent.h      — abstrakcyjny interfejs (create, getId, getHandle)
│   ├── SimpleWindow/       — okno główne (zarządza komponentami, WindowProc)
│   ├── Button/             — przycisk (onClick + onLongClick ≥ 800ms)
│   ├── Label/              — etykieta (wchar_t*, Unicode)
│   ├── Select/             — ComboBox (onChange, link do wektora)
│   ├── TextArea/           — edit multiline readonly (log/konsola)
│   ├── InputField/         — edit jednoliniowy (placeholder, password, maxLen)
│   ├── CheckBox/           — checkbox (onChange z bool)
│   ├── ProgressBar/        — pasek postępu (0-100%, Marquee)
│   ├── Chart/              — wykres czasu rzeczywistego (GDI, auto-scale)
│   ├── ValueDisplay/       — wyświetlacz LCD (GDI, double-buffered)
│   ├── ImageView/          — obrazy (GDI+, BMP/PNG/JPG, scale modes)
│   └── TabControl/         — zakładki z panelami
├── IO/
│   ├── Serial/             — COM port (wątkowy odbiór, auto-reconnect)
│   └── BLE/                — Bluetooth LE (SetupAPI, overlapped I/O)
└── Util/
    ├── StringUtils.*       — UTF-8 ↔ UTF-16 ↔ ANSI, extractComPort
    └── ConfigManager.*     — zapis/odczyt key=value (INI-like)
```

---

## Zakresy ID komponentów (auto-inkrementacja)

| Klasa | Początek ID |
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

## Wzorce tworzenia komponentów UI

### Ogólny wzorzec

```cpp
KomponentUI* component = new KomponentUI(x, y, w, h, ...parametry...);
window->add(component);  // create() wywoływane automatycznie
```

### Okno + komponenty

```cpp
SimpleWindow* window = new SimpleWindow(800, 600, "Tytuł", iconResId);
window->init();

// Komponenty dodawane przez add() — LifeTime zarządzany przez okno
window->add(new Label(x, y, w, h, L"Tekst"));
window->add(new Button(x, y, w, h, "Tekst", [](Button* b) { ... }));
```

> `SimpleWindow` jest właścicielem komponentów — destruktor je zwalnia.

### NIE usuwaj ręcznie komponentów dodanych do okna!

---

## Konwencja encodingu tekstu

| Komponent | Parametr | Encoding |
|-----------|----------|----------|
| **Label** | text | `const wchar_t*` (prefiks `L`) |
| **Button**, **Select**, **InputField**, **CheckBox**, **TabControl** | text | `const char*` (UTF-8) |
| **TextArea** | setText/append | Oba: `char*` (UTF-8) i `wchar_t*` (Unicode) |
| **ValueDisplay** | updateValue, setMode | `std::wstring` |

Konwersje: `StringUtils::utf8ToWide()` / `StringUtils::wideToUtf8()`

---

## Wzorce callbacków

### Button — kliknięcie i długie naciśnięcie

```cpp
new Button(x, y, w, h, "Tekst",
    [](Button* btn) { /* kliknięcie */ },
    [](Button* btn) { /* długie naciśnięcie ≥ 800ms */ }
);
```

### Select — zmiana wyboru

```cpp
new Select(x, y, w, h, "Domyślny",
    [](Select* sel) {
        const char* text = sel->getText();  // wybrany tekst
    }
);
```

### CheckBox — zmiana stanu

```cpp
new CheckBox(x, y, w, h, "Opcja", false,
    [](CheckBox* cb, bool checked) { /* ... */ }
);
```

### InputField — zmiana tekstu

```cpp
new InputField(x, y, w, h, "",
    [](InputField* field, const char* text) { /* ... */ }
);
```

### Serial — zdarzenia

```cpp
serial.onConnect([]() { /* ... */ });
serial.onDisconnect([]() { /* ... */ });
serial.onReceive([](const std::vector<uint8_t>& data) { /* ... */ });
```

### BLE — zdarzenia

```cpp
ble.onConnect([]() { /* ... */ });
ble.onDisconnect([]() { /* ... */ });
ble.onReceive([](const std::vector<uint8_t>& data) { /* ... */ });
ble.onDeviceDiscovered([](const BLEDevice& dev) { /* ... */ });
ble.onScanComplete([]() { /* ... */ });
ble.onError([](const std::wstring& msg) { /* ... */ });
```

---

## Komunikacja z urządzeniami

### Serial (COM)

```cpp
Serial serial;
serial.init();                       // Skanuje porty
serial.setPort("COM3");              // Ustaw port
serial.onReceive([](auto& data) {  // Callback odbioru
    // data = std::vector<uint8_t>
});
serial.connect();                    // Otwiera port + wątek odczytu
serial.write({0x55, 0xAA, 0x01});   // Wysyłanie
serial.disconnect();                 // Zamknij
```

### BLE (Bluetooth Low Energy)

```cpp
BLE ble;
ble.init();                          // Sprawdza adapter BT
ble.startScan(10);                   // Skanuj 10 sekund
ble.onScanComplete([&]() {
    auto& devices = ble.getDiscoveredDevices();
    if (!devices.empty()) {
        ble.connect(devices[0].address);
    }
});
ble.onReceive([](auto& data) { /* ... */ });
```

---

## Konfiguracja aplikacji

```cpp
ConfigManager config("settings.ini");

// Zapis
config.setValue("port", "COM3");

// Odczyt (z wartością domyślną)
std::string port = config.getValue("port", "COM1");

// Auto-save przy destrukcji obiektu
```

Format pliku:
```ini
# Komentarz
port=COM3
baudrate=9600
```

---

## Wskazówki dla Copilota

1. **Zawsze używaj C++17** (`-std=c++17`)
2. **Główne trzy funkcje:** `init()`, `setup()`, `loop()` — definiowane globalnie, nie w klasie
3. **Label przyjmuje `wchar_t*`** z prefiksem `L` — inne komponenty UTF-8 `char*`
4. **Nie usuwaj komponentów dodanych do SimpleWindow** — okno zarządza ich pamięcią
5. **Podaj `#include <Core.h>`** zawsze jako pierwszy include
6. **Callbacki:** Używaj lambd `[capture](Typ* ptr) { ... }`
7. **Nowe okno:** `new SimpleWindow(w, h, "title", 0); window->init();` — ZAWSZE `init()`!
8. **Serial/BLE:** Zawsze `init()` przed `connect()` / `startScan()`
9. **Konwersja stringów:** `StringUtils::utf8ToWide()` / `wideToUtf8()`
10. **Statyczny link:** Wynikowy `.exe` nie wymaga dodatkowych DLL
11. **Zasoby (ikona):** Plik `resources.rc` w katalogu głównym projektu, kompilowany automatycznie
12. **TextArea jest readonly** — do pól edycyjnych użyj `InputField`
13. **Chart** automatycznie usuwa stare dane i limituje FPS
14. **ValueDisplay** obsługuje double-buffering i nie miga
15. **TabControl::getTabPage()** zwraca HWND panelu — na nim umieszczaj kontrolki

### Typowy layout aplikacji

```
┌──────────────────────────────────────┐
│ Tytuł okna                     [_][□][X] │
├──────────────────────────────────────┤
│ Label: "Status"                      │
│ [Select ▼ COM3] [Połącz] [Rozłącz]  │
│ ┌──────────────────────────────────┐ │
│ │ TextArea (log/konsola)           │ │
│ │ > Odebrano: 55 AA 01 ...        │ │
│ │ > Wysłano: 55 AA 02 ...         │ │
│ └──────────────────────────────────┘ │
│ [Wyślij] [Wyczyść]                   │
└──────────────────────────────────────┘
```

### Minimalny `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
build_flags =
    -std=c++17
```

### Minimalny `main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(400, 300, "Hello JQB", 0);
    window->init();
    window->add(new Label(20, 20, 360, 30, L"Witaj!"));
}

void loop() {}
```
