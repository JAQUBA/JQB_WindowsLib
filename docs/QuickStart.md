# Quick Start — JQB_WindowsLib

Szybki przewodnik uruchomienia pierwszej aplikacji.

---

## 1. Wymagania

- [PlatformIO](https://platformio.org/) (jako rozszerzenie VS Code lub CLI)
- Kompilator **MinGW-w64 (GCC)** — PlatformIO pobiera go automatycznie
- Windows 10+ (x64)

---

## 2. Tworzenie projektu

### `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

> Flagi C++17, UNICODE, statyczne linkowanie i biblioteki są dodawane automatycznie przez bibliotekę.

### `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(500, 350, "Moja Aplikacja", 0);
    window->init();

    window->add(new Label(20, 20, 460, 30, L"Witaj w JQB_WindowsLib!"));

    window->add(new Button(20, 70, 150, 35, "Kliknij mnie", [](Button* btn) {
        MessageBoxW(NULL, L"Przycisk kliknięty!", L"Info", MB_OK);
    }));
}

void loop() {
    // Wywoływana w każdym cyklu pętli komunikatów — tu logika ciągła
}
```

### 3. Budowanie i uruchamianie

```bash
pio run                # Kompilacja
pio run --target exec  # Uruchomienie (jeśli skonfigurowane)
```

Wynikowy `.exe` znajdziesz w `.pio/build/app/program.exe`.

---

## 4. Dodawanie ikony (opcjonalnie)

1. Umieść plik `app.ico` w katalogu głównym projektu.
2. Utwórz `resources.rc`:
   ```rc
   101 ICON "app.ico"
   ```
3. W `SimpleWindow` podaj ID ikony:
   ```cpp
   window = new SimpleWindow(800, 600, "Tytuł", 101);
   ```

Skrypt `compile_resources.py` z biblioteki automatycznie skompiluje `.rc` do `.res`.

---

## 5. Cykl życia aplikacji

```
Program start
    │
    ▼
  init()          ← opcjonalnie: globalne zmienne, wczesna konfiguracja
    │
    ▼
  setup()         ← tworzenie okien, komponentów, inicjalizacja I/O
    │
    ▼
  ┌─── loop() ◄──┐
  │               │  ← pętla komunikatów Windows + loop()
  └───────────────┘
    │
    ▼
  Program end
```

- `init()` — wywoływana w konstruktorze `Core` (przed `WinMain`)
- `setup()` — wywoływana raz na starcie `WinMain`
- `loop()` — wywoływana w każdym cyklu pętli komunikatów

Wszystkie trzy funkcje są `__weak` — definiujesz tylko te, których potrzebujesz.

---

## 6. Częste wzorce

### Aktualizacja etykiety z loop()

```cpp
Label* lblTime;

void setup() {
    window = new SimpleWindow(400, 200, "Zegar", 0);
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

### Komunikacja Serial

```cpp
#include <IO/Serial/Serial.h>
Serial serial;

void setup() {
    // ... tworzenie okna ...
    serial.init();
    serial.setPort("COM3");
    serial.onReceive([](const std::vector<uint8_t>& data) {
        // przetwarzanie odebranych danych
    });
    serial.connect();
}
```

### Zapis konfiguracji

```cpp
#include <Util/ConfigManager.h>
ConfigManager config("settings.ini");

void setup() {
    std::string port = config.getValue("port", "COM1");
    // ... użycie port ...
}

// config automatycznie zapisze się przy zamknięciu (destruktor)
```

---

## 7. Następne kroki

- 📖 [Dokumentacja komponentów](.) — szczegóły każdego komponentu
- 💡 [Przykłady](examples/) — gotowe aplikacje do nauki
- 🔧 [Copilot Instructions](../.github/copilot-instructions.md) — po przeczytaniu Copilot pisze za Ciebie
