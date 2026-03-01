# JQB_WindowsLib

**Biblioteka C++ do tworzenia natywnych aplikacji Windows (WinAPI) w stylu Arduino-like.**

[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-0078d7.svg)]()
[![Build: PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange.svg)](https://platformio.org/)

---

## Spis treści

- [Opis](#opis)
- [Architektura](#architektura)
- [Quick Start](#quick-start)
- [Instalacja](#instalacja)
- [Struktura projektu](#struktura-projektu)
- [Komponenty UI](#komponenty-ui)
- [Moduły IO](#moduły-io)
- [Narzędzia (Util)](#narzędzia-util)
- [Cykl życia aplikacji](#cykl-życia-aplikacji)
- [Kompletny przykład](#kompletny-przykład)
- [API Reference](#api-reference)
- [FAQ](#faq)

---

## Opis

**JQB_WindowsLib** to biblioteka upraszczająca tworzenie natywnych aplikacji desktopowych Windows z GUI. Inspirowana stylem Arduino (`setup()` / `loop()`), ukrywa złożoność WinAPI za prostym, obiektowym interfejsem.

### Kluczowe cechy

| Cecha | Opis |
|-------|------|
| **Arduino-like API** | Funkcje `init()`, `setup()`, `loop()` — znany model programowania |
| **Gotowe komponenty UI** | 12 komponentów: okna, przyciski, etykiety, wykresy, zakładki... |
| **Komunikacja IO** | Serial (COM), Bluetooth Low Energy (BLE), USB HID |
| **Unicode / UTF-8** | Pełne wsparcie dla polskich znaków i Unicode |
| **Zarządzanie konfiguracją** | Zapis/odczyt ustawień w formacie INI |
| **PlatformIO Native** | Budowanie przez PlatformIO (`platform = native`) |
| **Statyczne linkowanie** | Wynikowy `.exe` nie wymaga dodatkowych DLL |

---

## Architektura

```
┌─────────────────────────────────────────────────────┐
│                    Twoja aplikacja                    │
│              init() → setup() → loop()               │
├─────────────────────────────────────────────────────┤
│                   JQB_WindowsLib                     │
│  ┌──────────┐  ┌──────────┐  ┌───────────────────┐  │
│  │   Core    │  │    UI    │  │        IO         │  │
│  │ WinMain  │  │ Window   │  │ Serial (COM)      │  │
│  │ MsgLoop  │  │ Button   │  │ BLE (Bluetooth)   │  │
│  │          │  │ Label    │  │ HID (USB HID)     │  │
│  │          │  │ Select   │  ├───────────────────┤  │
│  │          │  │ TextArea │  │       Util        │  │
│  │          │  │ Chart    │  │ StringUtils       │  │
│  │          │  │ CheckBox │  │ ConfigManager     │  │
│  │          │  │ InputField│ │                   │  │
│  │          │  │ ImageView│  │                   │  │
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

### Minimalny program — okno z przyciskiem

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>

SimpleWindow* window;
Label* label;

void setup() {
    window = new SimpleWindow(400, 300, "Moja aplikacja", 0);
    window->init();

    label = new Label(20, 20, 360, 30, L"Witaj świecie!");
    window->add(label);

    window->add(new Button(20, 60, 120, 35, "Kliknij mnie",
        [](Button* btn) {
            label->setText(L"Kliknięto przycisk!");
        }
    ));
}

void loop() {
    // Wywoływane cyklicznie w pętli komunikatów
}
```

### Konfiguracja `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

> **Uwaga:** Biblioteka automatycznie dodaje wymagane flagi: `-std=c++17`, `-DUNICODE`, `-D_UNICODE`, statyczne linkowanie, `subsystem:windows` oraz biblioteki `gdi32`/`comctl32`. Nie musisz ich deklarować ręcznie.

---

## Instalacja

### Przez PlatformIO (zalecane)

Dodaj do `platformio.ini`:

```ini
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

### Ręcznie

1. Sklonuj repo:
   ```bash
   git clone https://github.com/JAQUBA/JQB_WindowsLib.git
   ```
2. Umieść w katalogu `lib/` projektu PlatformIO lub wskaż ścieżkę w `lib_deps`.

### Wymagania

- **System:** Windows 10+ (x64)
- **Kompilator:** MinGW GCC (MinGW-w64 lub MinGW.org) — dostarczany przez PlatformIO
- **C++ Standard:** C++17 (dodawany automatycznie przez bibliotekę)
- **PlatformIO Core:** 6.x+

---

## Struktura projektu

```
JQB_WindowsLib/
├── library.json              # Manifest PlatformIO
├── scripts/
│   └── compile_resources.py  # Auto-konfiguracja buildu + kompilacja zasobów
└── src/
    ├── Core.h / Core.cpp     # Rdzeń — WinMain, pętla komunikatów
    ├── UI/                   # Komponenty interfejsu użytkownika
    │   ├── UIComponent.h     #   Interfejs bazowy (abstrakcyjny)
    │   ├── SimpleWindow/     #   Okno główne
    │   ├── Button/           #   Przycisk (click + long-press)
    │   ├── Label/            #   Etykieta tekstowa
    │   ├── Select/           #   Lista rozwijana (ComboBox)
    │   ├── TextArea/         #   Pole tekstowe wieloliniowe
    │   ├── InputField/       #   Pole edycji jednoliniowe
    │   ├── CheckBox/         #   Pole wyboru
    │   ├── ProgressBar/      #   Pasek postępu
    │   ├── Chart/            #   Wykres czasu rzeczywistego
    │   ├── ValueDisplay/     #   Wyświetlacz wartości (styl LCD)
    │   ├── ImageView/        #   Wyświetlanie obrazów (BMP/PNG/JPG)
    │   └── TabControl/       #   Kontrolka zakładek
    ├── IO/                   # Komunikacja
    │   ├── Serial/           #   Port szeregowy (COM)
    │   ├── BLE/              #   Bluetooth Low Energy
    │   └── HID/              #   USB HID (Feature Reports)
    └── Util/                 # Narzędzia
        ├── StringUtils.*     #   Konwersje UTF-8 ↔ UTF-16
        └── ConfigManager.*   #   Zapis/odczyt konfiguracji INI
```

---

## Komponenty UI

Wszystkie komponenty dziedziczą z `UIComponent` i implementują:
- `create(HWND parent)` — tworzy natywną kontrolkę WinAPI
- `getId()` — zwraca unikalny identyfikator
- `getHandle()` — zwraca uchwyt HWND

### Podsumowanie komponentów

| Komponent | Opis | Dokumentacja |
|-----------|------|--------------|
| **SimpleWindow** | Okno główne aplikacji | [docs/SimpleWindow.md](docs/SimpleWindow.md) |
| **Button** | Przycisk z onClick i onLongClick | [docs/Button.md](docs/Button.md) |
| **Label** | Etykieta tekstowa (Unicode) | [docs/Label.md](docs/Label.md) |
| **Select** | Lista rozwijana (ComboBox) | [docs/Select.md](docs/Select.md) |
| **TextArea** | Wieloliniowe pole tekstowe (readonly) | [docs/TextArea.md](docs/TextArea.md) |
| **InputField** | Pole edycji z placeholder i walidacją | [docs/InputField.md](docs/InputField.md) |
| **CheckBox** | Pole wyboru z callbackiem onChange | [docs/CheckBox.md](docs/CheckBox.md) |
| **ProgressBar** | Pasek postępu z trybem Marquee | [docs/ProgressBar.md](docs/ProgressBar.md) |
| **Chart** | Wykres pomiarów w czasie rzeczywistym | [docs/Chart.md](docs/Chart.md) |
| **ValueDisplay** | Wyświetlacz wartości w stylu LCD | [docs/ValueDisplay.md](docs/ValueDisplay.md) |
| **ImageView** | Obrazy z pliku, zasobów lub pamięci | [docs/ImageView.md](docs/ImageView.md) |
| **TabControl** | Kontrolka zakładek z panelami | [docs/TabControl.md](docs/TabControl.md) |

---

## Moduły IO

| Moduł | Opis | Dokumentacja |
|-------|------|--------------|
| **Serial** | Komunikacja przez port COM (RS-232) | [docs/Serial.md](docs/Serial.md) |
| **BLE** | Bluetooth Low Energy — skanowanie i komunikacja | [docs/BLE.md](docs/BLE.md) |
| **HID** | USB HID — Feature Reports, enumeracja urządzeń | [docs/HID.md](docs/HID.md) |

---

## Narzędzia (Util)

| Moduł | Opis | Dokumentacja |
|-------|------|--------------|
| **StringUtils** | Konwersje UTF-8 ↔ UTF-16/ANSI | [docs/StringUtils.md](docs/StringUtils.md) |
| **ConfigManager** | Menedżer konfiguracji (format key=value) | [docs/ConfigManager.md](docs/ConfigManager.md) |

---

## Cykl życia aplikacji

```
Program Start
     │
     ▼
  Core() constructor   ← automatyczny, tworzy globalny obiekt _core
     │
     ▼
  init()              ← __weak — można nadpisać; wywoływana w konstruktorze Core
     │
     ▼
  WinMain()           ← punkt wejścia Windows
     │
     ▼
  setup()             ← tworzenie okien, komponentów, połączeń
     │
     ▼
  ┌─── Pętla komunikatów (GetMessage → TranslateMessage → DispatchMessage) ───┐
  │                                                                            │
  │   loop()  ← wywoływana po każdym cyklu pętli komunikatów                 │
  │                                                                            │
  └────────────────────────── WM_QUIT kończy pętlę ──────────────────────────┘
     │
     ▼
  Program End
```

### Trzy funkcje callback (opcjonalne, `__weak`)

| Funkcja | Kiedy | Typowe zastosowanie |
|---------|-------|---------------------|
| `init()` | Przed `WinMain`, w konstruktorze `Core` | Wczesna inicjalizacja |
| `setup()` | Po `WinMain`, przed pętlą komunikatów | Tworzenie GUI, nawiązanie połączeń |
| `loop()` | W każdym cyklu pętli komunikatów | Aktualizacja stanu, polling |

---

## Kompletny przykład

### Aplikacja z oknem, przyciskami, polem Select i komunikacją Serial

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
    // Inicjalizacja okna
    window = new SimpleWindow(600, 400, "Serial Monitor", 0);
    window->init();

    // Status
    lblStatus = new Label(20, 10, 560, 20, L"Rozłączono");
    window->add(lblStatus);

    // Wybór portu COM
    serial.init();
    selPort = new Select(20, 40, 200, 25, "Wybierz port", [](Select* sel) {
        serial.setPort(sel->getText());
    });
    // Wypełnij listę portów
    for (const auto& port : serial.getAvailablePorts()) {
        selPort->addItem(port.c_str());
    }
    window->add(selPort);

    // Przycisk Połącz
    window->add(new Button(230, 40, 100, 25, "Połącz", [](Button* btn) {
        if (serial.isConnected()) {
            serial.disconnect();
        } else {
            serial.connect();
        }
    }));

    // Log komunikacji
    textLog = new TextArea(20, 80, 560, 280);
    window->add(textLog);

    // Callback odbierania danych
    serial.onReceive([](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        textLog->append(text);
    });

    serial.onConnect([]() {
        lblStatus->setText(L"Połączono!");
    });

    serial.onDisconnect([]() {
        lblStatus->setText(L"Rozłączono");
    });
}

void loop() {
    // pętla komunikatów obsługuje zdarzenia automatycznie
}
```

---

## API Reference

Pełna dokumentacja API każdego komponentu znajduje się w katalogu [docs/](docs/):

### Poradniki

- **[Quick Start](docs/QuickStart.md)** — szybkie uruchomienie pierwszej aplikacji
- **[Przykłady](docs/examples/)** — 7 gotowych aplikacji demonstracyjnych

### Komponenty

- [Core](docs/Core.md) — rdzeń aplikacji
- [SimpleWindow](docs/SimpleWindow.md) — okno główne
- [Button](docs/Button.md) — przycisk
- [Label](docs/Label.md) — etykieta
- [Select](docs/Select.md) — lista rozwijana
- [TextArea](docs/TextArea.md) — pole tekstowe
- [InputField](docs/InputField.md) — pole edycji
- [CheckBox](docs/CheckBox.md) — pole wyboru
- [ProgressBar](docs/ProgressBar.md) — pasek postępu
- [Chart](docs/Chart.md) — wykres
- [ValueDisplay](docs/ValueDisplay.md) — wyświetlacz wartości
- [ImageView](docs/ImageView.md) — wyświetlacz obrazów
- [TabControl](docs/TabControl.md) — zakładki

### IO & Util

- [Serial](docs/Serial.md) — komunikacja COM
- [BLE](docs/BLE.md) — Bluetooth Low Energy
- [HID](docs/HID.md) — USB HID (Feature Reports)
- [StringUtils](docs/StringUtils.md) — konwersje stringów
- [ConfigManager](docs/ConfigManager.md) — menedżer konfiguracji

---

## FAQ

### Jak dodać ikonę do aplikacji?

1. Stwórz plik `resources.rc` w katalogu głównym projektu:
   ```rc
   1 ICON "icon.ico"
   ```
2. Umieść plik `icon.ico` obok `resources.rc`.
3. Skrypt `compile_resources.py` automatycznie skompiluje zasoby podczas budowania.
4. Użyj `SimpleWindow(width, height, "Tytuł", 1)` — ostatni parametr to ID ikony.

### Jak obsługiwać polskie znaki?

Biblioteka w pełni obsługuje Unicode. Używaj:
- `Label` przyjmuje `const wchar_t*` — prefiks `L`: `L"Zażółć gęślą jaźń"`
- `Button`, `Select` i inne przyjmują `const char*` w UTF-8
- `StringUtils::utf8ToWide()` i `StringUtils::wideToUtf8()` do konwersji

### Czy mogę użyć bez PlatformIO?

Tak — biblioteka to czysty C++ z WinAPI. Wymaga linkowania: `gdi32`, `comctl32`. Moduły IO dynamicznie ładują: `setupapi`, `gdiplus`, `shlwapi`, `bthprops`, `hid`. Kompilacja: `g++ -std=c++17 -DUNICODE -D_UNICODE -mwindows -lgdi32 -lcomctl32 ...`

### Jak stworzyć własny komponent UI?

Dziedzicz z `UIComponent`:
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

## Licencja

[GPL-3.0-or-later](https://www.gnu.org/licenses/gpl-3.0.html)

**Autor:** [JAQUBA](https://github.com/JAQUBA)
