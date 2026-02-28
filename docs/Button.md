# Button — Przycisk

> `#include <UI/Button/Button.h>`

## Opis

Przycisk z obsługą:
- **Kliknięcia** (`onClick`) — callback wywoływany po krótkim naciśnięciu
- **Długiego naciśnięcia** (`onLongClick`) — callback po przytrzymaniu ≥ 800 ms

## Konstruktor

```cpp
Button(int x, int y, int width, int height, const char* text,
       std::function<void(Button*)> onClick,
       std::function<void(Button*)> onLongClick = nullptr);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja lewego górnego rogu (piksele, względem rodzica) |
| `width`, `height` | `int` | Rozmiar przycisku |
| `text` | `const char*` | Tekst na przycisku (UTF-8) |
| `onClick` | `function<void(Button*)>` | Callback kliknięcia |
| `onLongClick` | `function<void(Button*)>` | Callback długiego naciśnięcia (opcjonalny) |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę Windows (wywoływana automatycznie przez `SimpleWindow::add()`) |
| `handleClick()` | `void` | Wywołuje callback `onClick` |
| `handleLongClick()` | `void` | Wywołuje callback `onLongClick` |
| `getX()` | `int` | Pozycja X |
| `getY()` | `int` | Pozycja Y |
| `getWidth()` | `int` | Szerokość |
| `getHeight()` | `int` | Wysokość |
| `getText()` | `const char*` | Tekst przycisku |
| `getHandle()` | `HWND` | Uchwyt kontrolki Windows |
| `getId()` | `int` | Unikalny identyfikator (auto-generowany od 1000) |

## Mechanizm Long Press

- Przycisk przechwytuje `WM_LBUTTONDOWN` / `WM_LBUTTONUP` przez podklasowanie (subclassing)
- Timer co 50 ms sprawdza czas naciśnięcia
- Jeśli ≥ 800 ms → `onLongClick` i **nie** wywołuje `onClick`
- Jeśli < 800 ms → tylko `onClick`

## Przykłady

### Przycisk z kliknięciem

```cpp
window->add(new Button(20, 20, 120, 35, "Kliknij",
    [](Button* btn) {
        // Obsługa kliknięcia
    }
));
```

### Przycisk z długim naciśnięciem

```cpp
window->add(new Button(20, 60, 120, 35, "Przytrzymaj",
    [](Button* btn) {
        // Krótkie kliknięcie
    },
    [](Button* btn) {
        // Długie naciśnięcie (≥ 800 ms)
    }
));
```

### Przycisk akcji z dostępem do innych komponentów

```cpp
Label* status = new Label(20, 20, 200, 25, L"Gotowy");
window->add(status);

window->add(new Button(20, 60, 100, 30, "Akcja",
    [status](Button* btn) {
        status->setText(L"Wykonano akcję!");
    }
));
```

## Uwagi

- ID przycisków zaczynają się od **1000** (auto-inkrementacja)
- Tekst przycisku jest konwertowany z UTF-8 na UTF-16 automatycznie
- Destruktor automatycznie czyści mapy wewnętrzne i niszczy okno
