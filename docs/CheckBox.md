# CheckBox — Pole wyboru

> `#include <UI/CheckBox/CheckBox.h>`

## Opis

Pole wyboru (checkbox) z callbackiem `onChange`. Stan zmienia się automatycznie przy kliknięciu (styl `BS_AUTOCHECKBOX`).

## Konstruktor

```cpp
CheckBox(int x, int y, int width, int height, const char* text,
         bool checked = false,
         std::function<void(CheckBox*, bool)> onChange = nullptr);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar (w tym tekst obok) |
| `text` | `const char*` | Tekst etykiety (UTF-8) |
| `checked` | `bool` | Stan początkowy |
| `onChange` | `function<void(CheckBox*, bool)>` | Callback ze stanem |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę |
| `setText(const char* text)` | `void` | Zmienia tekst |
| `getText()` | `std::string` | Pobiera tekst |
| `setChecked(bool checked)` | `void` | Ustawia stan programowo |
| `isChecked()` | `bool` | Sprawdza stan |
| `handleClick()` | `void` | Obsługa kliknięcia |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 3000) |

## Przykłady

### Prosty checkbox

```cpp
window->add(new CheckBox(20, 50, 200, 25, "Włącz powiadomienia", true,
    [](CheckBox* cb, bool checked) {
        if (checked) {
            // Włączono
        } else {
            // Wyłączono
        }
    }
));
```

### Programowe sterowanie

```cpp
CheckBox* cbAuto = new CheckBox(20, 50, 200, 25, "Tryb automatyczny", false);
window->add(cbAuto);

// Później:
cbAuto->setChecked(true);     // Zaznacz
bool state = cbAuto->isChecked();  // Odczytaj stan
```

## Uwagi

- ID zaczynają się od **3000**
- Styl: `BS_AUTOCHECKBOX` — Windows automatycznie przełącza stan
- `isChecked()` odczytuje rzeczywisty stan kontrolki przez `BM_GETCHECK`
