# Przykład 01 — Hello Window

Minimalna aplikacja z oknem, etykietą i przyciskiem.

## `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>

SimpleWindow* window;
Label* lblInfo;
int clickCount = 0;

void setup() {
    window = new SimpleWindow(450, 250, "Hello JQB_WindowsLib", 0);
    window->init();

    window->add(new Label(20, 20, 410, 30, L"Witaj! To jest Twoja pierwsza aplikacja."));

    lblInfo = new Label(20, 60, 410, 25, L"Kliknięć: 0");
    window->add(lblInfo);

    window->add(new Button(20, 100, 180, 35, "Kliknij mnie!", [](Button* btn) {
        clickCount++;
        wchar_t buf[64];
        swprintf(buf, 64, L"Kliknięć: %d", clickCount);
        lblInfo->setText(buf);
    }));

    window->add(new Button(220, 100, 180, 35, "Resetuj", [](Button* btn) {
        clickCount = 0;
        lblInfo->setText(L"Kliknięć: 0");
    }));
}

void loop() {
    // Pusta — aplikacja jest w pełni zdarzeniowa
}
```

## Kluczowe punkty

1. **SimpleWindow** tworzy główne okno — parametry: szerokość, wysokość, tytuł (UTF-8), ID ikony (0 = brak)
2. **Zawsze wywołaj `window->init()`** po utworzeniu okna
3. **Label** przyjmuje tekst jako `const wchar_t*` (prefiks `L`)
4. **Button** przyjmuje tekst jako `const char*` (UTF-8) i callback `std::function<void(Button*)>`
5. Komponenty dodane przez `add()` są zarządzane przez okno — **nie usuwaj ich ręcznie**
6. `swprintf` do formatowania tekstu wide-string (zamiast `printf`)
