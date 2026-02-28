# StringUtils — Konwersje stringów

> `#include <Util/StringUtils.h>`

## Opis

Namespace z funkcjami konwersji encodingu stringów. Wykorzystuje Windows API (`MultiByteToWideChar` / `WideCharToMultiByte`) dla niezawodnej konwersji.

## Funkcje

### `utf8ToWide`

```cpp
std::wstring StringUtils::utf8ToWide(const std::string& utf8);
```

Konwertuje string UTF-8 na UTF-16 (`wstring`). Używa `CP_UTF8`.

```cpp
std::string utf8 = "Zażółć gęślą jaźń";
std::wstring wide = StringUtils::utf8ToWide(utf8);
```

### `wideToUtf8`

```cpp
std::string StringUtils::wideToUtf8(const std::wstring& wstr);
```

Konwertuje UTF-16 (`wstring`) na UTF-8. Używa `CP_UTF8`.

```cpp
std::wstring wide = L"Napięcie: 12.5 V";
std::string utf8 = StringUtils::wideToUtf8(wide);
```

### `wstringToString`

```cpp
std::string StringUtils::wstringToString(const std::wstring& wstr);
```

Alias dla `wideToUtf8()`.

### `ansiToWide`

```cpp
std::wstring StringUtils::ansiToWide(const std::string& ansi);
```

Konwertuje string w kodowaniu ANSI (stronę kodową systemu) na UTF-16. Używa `CP_ACP`.

```cpp
std::string ansi = "Tekst w ANSI";
std::wstring wide = StringUtils::ansiToWide(ansi);
```

### `extractComPort`

```cpp
std::string StringUtils::extractComPort(const char* text);
```

Wyodrębnia nazwę portu COM z tekstu, np.:

```cpp
StringUtils::extractComPort("USB Serial Port (COM3)");   // → "COM3"
StringUtils::extractComPort("COM12 - Arduino");           // → "COM12"
StringUtils::extractComPort("No port here");              // → ""
```

## Kiedy użyć

| Konwersja | Funkcja | Przykład |
|-----------|---------|---------|
| UTF-8 → WinAPI | `utf8ToWide()` | Button text → `CreateWindowW()` |
| WinAPI → UTF-8 | `wideToUtf8()` | `GetWindowTextW()` → `std::string` |
| ANSI → WinAPI | `ansiToWide()` | Fallback dla starych danych |
| Friendly Name → COM | `extractComPort()` | Enumeracja portów |

## Uwagi

- Wszystkie funkcje zwracają pusty string przy błędzie konwersji lub pustym wejściu
- Wyjątki C++ przechwytywane wewnętrznie (catch-all → pusty string)
- Używane wewnętrznie przez prawie wszystkie komponenty UI i IO
