# SimpleWindow — Okno główne aplikacji

> `#include <UI/SimpleWindow/SimpleWindow.h>`

## Opis

`SimpleWindow` to główne okno aplikacji. Zarządza tworzeniem okna Windows, pętlą komunikatów i kolekcją komponentów UI. Obsługuje zdarzenia kliknięć, zmianę wyboru, powiadomienia kontrolek i timer dla długich naciśnięć przycisków.

## Konstruktor

```cpp
SimpleWindow(int width, int height, const char* title, int iconId);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `width` | `int` | Szerokość okna w pikselach (obszar klienta) |
| `height` | `int` | Wysokość okna w pikselach (obszar klienta) |
| `title` | `const char*` | Tytuł okna (UTF-8 — obsługuje polskie znaki) |
| `iconId` | `int` | ID ikony z zasobów (`resources.rc`). Podaj `0` dla braku ikony. |

## Metody

### `bool init()`

Rejestruje klasę okna, tworzy okno i wyświetla je. **Musi być wywołana po konstruktorze.**

```cpp
SimpleWindow* win = new SimpleWindow(800, 600, "Moja Apka", 1);
if (!win->init()) {
    // Błąd tworzenia okna
}
```

**Zwraca:** `true` jeśli okno zostało utworzone, `false` przy błędzie.

### `void add(UIComponent* component)`

Dodaje dowolny komponent UI do okna. Komponent jest automatycznie tworzony (`create()`) z uchwytem okna jako rodzicem.

```cpp
Button* btn = new Button(10, 10, 100, 30, "OK", [](Button*){});
win->add(btn);
```

### Przeciążenia `add()` (kompatybilność wsteczna)

```cpp
void add(Button* button);
void add(Label* label);
void add(Select* select);
void add(TextArea* textArea);
void add(ValueDisplay* valueDisplay);
void add(Chart* chart);
```

### `void close()`

Zamyka okno — wysyła `WM_CLOSE`.

```cpp
win->close();
```

## Zarządzanie pamięcią

`SimpleWindow` posiada (*owns*) wszystkie dodane komponenty. Destruktor automatycznie zwalnia:
- Wszystkie komponenty z wektora `m_components` (`delete`)
- Okno Windows (`DestroyWindow`)
- Wysyła `PostQuitMessage(0)` — kończy pętlę komunikatów

> **Ważne:** Nie usuwaj ręcznie komponentów dodanych do okna!

## Obsługa zdarzeń (internals)

`WindowProc` automatycznie obsługuje:

| Komunikat | Obsługa |
|-----------|---------|
| `WM_COMMAND` + `BN_CLICKED` | `component->handleClick()` |
| `WM_COMMAND` + `BN_PUSHED` | `startButtonPress()` → long press |
| `WM_COMMAND` + `CBN_SELCHANGE` | `component->handleSelection()` |
| `WM_NOTIFY` + `TCN_SELCHANGE` | `component->handleSelection()` (TabControl) |
| `WM_TIMER` | `checkForLongPresses()` |
| `WM_CLOSE` | `DestroyWindow()` |
| `WM_DESTROY` | `PostQuitMessage(0)` |

## Przykład

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/Select/Select.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(500, 350, "Demo Window", 0);
    window->init();

    // Etykieta
    window->add(new Label(20, 20, 200, 25, L"Wybierz opcję:"));

    // Lista rozwijana
    Select* sel = new Select(20, 50, 200, 25, "Opcja 1", [](Select* s) {
        // reakcja na zmianę wyboru
    });
    sel->addItem("Opcja 1");
    sel->addItem("Opcja 2");
    sel->addItem("Opcja 3");
    window->add(sel);

    // Przycisk zamykający
    window->add(new Button(20, 100, 120, 35, "Zamknij", [](Button*) {
        window->close();
    }));
}

void loop() {}
```

## Uwagi

- Okno używa stylu `WS_OVERLAPPEDWINDOW` (z przyciskami minimalizacji, maksymalizacji, zamknięcia).
- Rozmiar podany w konstruktorze to rozmiar **obszaru klienta** (client area). Rzeczywisty rozmiar okna jest większy o ramkę i pasek tytułowy (`AdjustWindowRect`).
- Aktualnie wspierany jest **jeden obiekt** `SimpleWindow` (instancja singleton via `s_instance`).
