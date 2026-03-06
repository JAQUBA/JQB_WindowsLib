# StringUtils — String Conversions

> `#include <Util/StringUtils.h>`

## Description

Namespace with string encoding conversion functions. Uses Windows API (`MultiByteToWideChar` / `WideCharToMultiByte`) for reliable conversion.

## Functions

### `utf8ToWide`

```cpp
std::wstring StringUtils::utf8ToWide(const std::string& utf8);
```

Converts a UTF-8 string to UTF-16 (`wstring`). Uses `CP_UTF8`.

```cpp
std::string utf8 = "Temperature: 25.3\xC2\xB0""C";
std::wstring wide = StringUtils::utf8ToWide(utf8);
```

### `wideToUtf8`

```cpp
std::string StringUtils::wideToUtf8(const std::wstring& wstr);
```

Converts UTF-16 (`wstring`) to UTF-8. Uses `CP_UTF8`.

```cpp
std::wstring wide = L"Voltage: 12.5 V";
std::string utf8 = StringUtils::wideToUtf8(wide);
```

### `wstringToString`

```cpp
std::string StringUtils::wstringToString(const std::wstring& wstr);
```

Alias for `wideToUtf8()`.

### `ansiToWide`

```cpp
std::wstring StringUtils::ansiToWide(const std::string& ansi);
```

Converts an ANSI string (system code page) to UTF-16. Uses `CP_ACP`.

```cpp
std::string ansi = "ANSI text";
std::wstring wide = StringUtils::ansiToWide(ansi);
```

### `extractComPort`

```cpp
std::string StringUtils::extractComPort(const char* text);
```

Extracts a COM port name from text, e.g.:

```cpp
StringUtils::extractComPort("USB Serial Port (COM3)");   // → "COM3"
StringUtils::extractComPort("COM12 - Arduino");           // → "COM12"
StringUtils::extractComPort("No port here");              // → ""
```

## When to Use

| Conversion | Function | Example |
|------------|----------|---------|
| UTF-8 → WinAPI | `utf8ToWide()` | Button text → `CreateWindowW()` |
| WinAPI → UTF-8 | `wideToUtf8()` | `GetWindowTextW()` → `std::string` |
| ANSI → WinAPI | `ansiToWide()` | Fallback for legacy data |
| Friendly Name → COM | `extractComPort()` | Port enumeration |

## Notes

- All functions return an empty string on conversion error or empty input
- C++ exceptions are caught internally (catch-all → empty string)
- Used internally by almost all UI and IO components
