# Label — Etykieta tekstowa

> `#include <UI/Label/Label.h>`

## Opis

Statyczna etykieta tekstowa. Wyświetla tekst w formacie Unicode (UTF-16, `wchar_t*`). Idealna do wyświetlania statusów, opisów i nagłówków.

## Konstruktor

```cpp
Label(int x, int y, int width, int height, const wchar_t* text);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja lewego górnego rogu |
| `width`, `height` | `int` | Rozmiar etykiety |
| `text` | `const wchar_t*` | Tekst etykiety (Unicode, prefiks `L`) |

> **Uwaga:** Label przyjmuje `wchar_t*` (nie `char*` jak inne komponenty). Używaj prefiksu `L`:

```cpp
new Label(10, 10, 200, 20, L"Zażółć gęślą jaźń");
```

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę `STATIC` |
| `setText(const wchar_t* text)` | `void` | Zmienia tekst etykiety |
| `getText()` | `const wchar_t*` | Pobiera aktualny tekst |
| `getX()`, `getY()` | `int` | Pozycja |
| `getWidth()`, `getHeight()` | `int` | Rozmiar |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 2000) |

## Przykłady

### Podstawowa etykieta

```cpp
Label* lbl = new Label(20, 10, 300, 25, L"Status: Oczekuje...");
window->add(lbl);
```

### Dynamiczna zmiana tekstu

```cpp
Label* lblResult = new Label(20, 10, 300, 25, L"");
window->add(lblResult);

// Później w programie:
lblResult->setText(L"Połączono z urządzeniem!");
lblResult->setText(L"Wartość: 123.45 V");
```

### Z konwersją UTF-8 → wchar_t

```cpp
#include <Util/StringUtils.h>

std::string utf8Text = "Napiecie: 12.5V";
std::wstring wideText = StringUtils::utf8ToWide(utf8Text);
label->setText(wideText.c_str());
```

## Uwagi

- Styl: `WS_CHILD | WS_VISIBLE | SS_LEFT` — tekst wyrównany do lewej
- ID zaczynają się od **2000**
- Etykieta nie obsługuje zdarzeń kliknięcia
