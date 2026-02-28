# Core — Rdzeń aplikacji

> `#include <Core.h>`

## Opis

`Core` to rdzeń biblioteki JQB_WindowsLib. Zapewnia:
- Punkt wejścia aplikacji Windows (`WinMain`)
- Pętlę komunikatów (message loop)
- Model programowania w stylu Arduino: `init()` → `setup()` → `loop()`

## Jak to działa

```
1. Globalny obiekt `_core` jest tworzony automatycznie
2. Konstruktor Core wywołuje init()
3. Windows wywołuje WinMain()
4. WinMain wywołuje setup(), potem uruchamia pętlę komunikatów
5. W każdym cyklu pętli wywoływana jest loop()
6. WM_QUIT kończy pętlę i zamyka aplikację
```

## Klasa `Core`

```cpp
class Core {
public:
    HINSTANCE hInstance;       // Uchwyt instancji aplikacji
    HINSTANCE hPrevInstance;   // Uchwyt poprzedniej instancji (zawsze NULL)
    LPSTR     lpCmdLine;      // Argumenty wiersza poleceń
    int       nCmdShow;       // Tryb wyświetlania okna (SW_SHOW, etc.)
    
    Core();                   // Konstruktor — wywołuje init()
};

extern Core _core;            // Globalny obiekt — dostępny wszędzie
```

## Funkcje callback

Wszystkie trzy funkcje są opcjonalne (oznaczone `__weak`). Jeśli ich nie zdefiniujesz, użyte zostaną puste implementacje domyślne.

### `void init()`

Wywoływana w konstruktorze `Core`, **przed** `WinMain`. Służy do bardzo wczesnej inicjalizacji.

```cpp
void init() {
    // np. konfiguracja zmiennych globalnych
}
```

### `void setup()`

Wywoływana raz, na początku `WinMain`, **przed** pętlą komunikatów. Idealne miejsce na:
- Tworzenie okien
- Dodawanie komponentów UI
- Nawiązywanie połączeń (Serial, BLE)

```cpp
void setup() {
    SimpleWindow* window = new SimpleWindow(800, 600, "My App", 0);
    window->init();
    // ... dodawanie komponentów
}
```

### `void loop()`

Wywoływana w każdym cyklu pętli komunikatów, **po** przetworzeniu wiadomości Windows.

```cpp
void loop() {
    // aktualizacja stanu, polling czujników, itp.
}
```

> **Uwaga:** `loop()` jest wywoływana po każdym `DispatchMessage()`, nie w stałych interwałach czasowych. Częstotliwość zależy od ilości komunikatów w kolejce.

## Pętla komunikatów (internals)

```cpp
int WINAPI WinMain(HINSTANCE hInstance, ...) {
    _core.hInstance = hInstance;
    // ...
    setup();
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        loop();
    }
    return (int)msg.wParam;
}
```

## Dostęp do instancji

Globalny obiekt `_core` jest dostępny z dowolnego pliku:

```cpp
#include <Core.h>

void mojaFunkcja() {
    HINSTANCE hInst = _core.hInstance;  // Potrzebne np. do CreateWindow
}
```

## Przykład kompletnej aplikacji

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>

SimpleWindow* window;
Label* label;
int counter = 0;

void init() {
    // Wczesna inicjalizacja — opcjonalna
}

void setup() {
    window = new SimpleWindow(300, 200, "Counter App", 0);
    window->init();
    
    label = new Label(20, 20, 260, 30, L"Counter: 0");
    window->add(label);
}

void loop() {
    // UWAGA: loop() jest zależna od komunikatów Windows
    // Dla operacji cyklicznych lepiej użyć SetTimer w WinAPI
}
```
