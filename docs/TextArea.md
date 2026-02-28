# TextArea — Wieloliniowe pole tekstowe

> `#include <UI/TextArea/TextArea.h>`

## Opis

Wieloliniowe pole tekstowe, domyślnie **tylko do odczytu** (`ES_READONLY`). Idealne jako log komunikacji, konsola wyjściowa lub wyświetlacz danych.

## Konstruktor

```cpp
TextArea(int x, int y, int width, int height);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę EDIT (multiline, readonly) |
| `setText(const char* text)` | `void` | Ustawia tekst (UTF-8) |
| `setText(const std::string& text)` | `void` | Ustawia tekst (UTF-8 string) |
| `setText(const wchar_t* text)` | `void` | Ustawia tekst (Unicode) |
| `setText(const std::wstring& text)` | `void` | Ustawia tekst (Unicode wstring) |
| `append(const std::string& text)` | `void` | Dopisuje tekst na końcu (UTF-8) |
| `append(const std::wstring& text)` | `void` | Dopisuje tekst na końcu (Unicode) |
| `clear()` | `void` | Czyści zawartość |
| `getText()` | `const wchar_t*` | Pobiera tekst (Unicode) |
| `getTextUTF8()` | `std::string` | Pobiera tekst jako UTF-8 |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 4000) |

## Przykłady

### Log komunikacji Serial

```cpp
TextArea* logArea = new TextArea(20, 80, 560, 300);
window->add(logArea);

serial.onReceive([logArea](const std::vector<uint8_t>& data) {
    std::string text(data.begin(), data.end());
    logArea->append(text);
});
```

### Wyświetlacz z auto-scrollem

```cpp
TextArea* ta = new TextArea(10, 10, 400, 200);
window->add(ta);

ta->append("Linia 1\r\n");
ta->append("Linia 2\r\n");
ta->append("Linia 3\r\n");  // automatyczny scroll do dołu
```

### Ustawianie całego tekstu

```cpp
ta->setText("Nowa zawartość\r\nDruga linia");
ta->clear();  // Czyści wszystko
```

## Uwagi

- **Styl:** `ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY`
- Nowe linie: używaj `\r\n` (Windows convention)
- `append()` automatycznie przewija do końca (`SB_BOTTOM`)
- Kodowanie: wewnętrznie Unicode (`std::wstring`). Metody `setText/append` z `const char*` / `std::string` konwertują z UTF-8
- ID zaczynają się od **4000**
