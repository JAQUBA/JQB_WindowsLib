# InputField — Pole edycji

> `#include <UI/InputField/InputField.h>`

## Opis

Jednoliniowe pole edycji tekstu z obsługą:
- Tekst zastępczy (placeholder)
- Limit znaków
- Tryb readonly
- Tryb hasła
- Callback `onTextChange`

## Konstruktor

```cpp
InputField(int x, int y, int width, int height,
           const char* defaultText = "",
           std::function<void(InputField*, const char*)> onTextChange = nullptr);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar |
| `defaultText` | `const char*` | Tekst startowy (UTF-8) |
| `onTextChange` | `function<void(InputField*, const char*)>` | Callback przy zmianie lub Enter |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę EDIT |
| `setText(const char* text)` | `void` | Ustawia tekst (UTF-8) |
| `getText()` | `std::string` | Pobiera tekst (UTF-8) |
| `setPlaceholder(const char* text)` | `void` | Tekst zastępczy (placeholder, `EM_SETCUEBANNER`) |
| `setMaxLength(int maxLength)` | `void` | Limit znaków |
| `setReadOnly(bool readOnly)` | `void` | Tryb tylko do odczytu |
| `setPassword(bool isPassword)` | `void` | Tryb hasła (maskowanie znaków) |
| `handleTextChange()` | `void` | Wywołuje callback |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 2000) |

## Przykłady

### Pole z placeholderem

```cpp
InputField* input = new InputField(20, 50, 200, 25, "");
input->setPlaceholder("Wpisz nazwę...");
window->add(input);
```

### Pole z callbackiem

```cpp
InputField* search = new InputField(20, 50, 200, 25, "",
    [](InputField* field, const char* text) {
        // Reaguj na zmianę tekstu lub Enter
    }
);
window->add(search);
```

### Pole hasła z limitem

```cpp
InputField* password = new InputField(20, 100, 200, 25, "");
password->setPassword(true);
password->setMaxLength(32);
password->setPlaceholder("Haslo...");
window->add(password);
```

### Odczyt wartości

```cpp
std::string value = inputField->getText();
```

## Uwagi

- ID zaczynają się od **2000**
- Styl: `WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL`
- Podklasowanie (subclassing) przechwytuje `WM_CHAR` (Enter) i `WM_COMMAND` (EN_CHANGE)
- Placeholder wymaga Common Controls 6.0 (domyślnie dostępne w Windows Vista+)
