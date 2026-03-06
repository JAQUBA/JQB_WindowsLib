# TextArea — Multiline Text Field

> `#include <UI/TextArea/TextArea.h>`

## Description

Multiline text field, **readonly** by default (`ES_READONLY`). Ideal as a communication log, output console, or data display. Supports font and color styling.

## Constructor

```cpp
TextArea(int x, int y, int width, int height);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size |

## Methods

### Text Operations

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the EDIT control (multiline, readonly) |
| `setText(const char* text)` | `void` | Sets text (UTF-8) |
| `setText(const std::string& text)` | `void` | Sets text (UTF-8 string) |
| `setText(const wchar_t* text)` | `void` | Sets text (Unicode) |
| `setText(const std::wstring& text)` | `void` | Sets text (Unicode wstring) |
| `append(const std::string& text)` | `void` | Appends text (UTF-8) |
| `append(const std::wstring& text)` | `void` | Appends text (Unicode) |
| `clear()` | `void` | Clears content |
| `getText()` | `const wchar_t*` | Gets text (Unicode) |
| `getTextUTF8()` | `std::string` | Gets text as UTF-8 |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 4000) |

### Font and Color Styling

| Method | Returns | Description |
|--------|---------|-------------|
| `setFont(const wchar_t* fontName, int size, bool bold = false, bool italic = false)` | `void` | Sets font (name, size, bold, italic) |
| `setTextColor(COLORREF color)` | `void` | Sets text color |
| `setBackColor(COLORREF color)` | `void` | Sets background color |
| `hasCustomColors()` | `bool` | Whether custom colors are set |
| `getTextColor()` | `COLORREF` | Current text color |
| `getBackColor()` | `COLORREF` | Current background color |
| `getBackBrush()` | `HBRUSH` | Background brush (used by SimpleWindow) |

> **Note:** Colors are applied via `WM_CTLCOLORSTATIC` handled automatically by `SimpleWindow`. The TextArea must be added to the window (`window->add()`) before setting colors.

## Examples

### Communication Log

```cpp
TextArea* logArea = new TextArea(20, 80, 560, 300);
window->add(logArea);

serial.onReceive([logArea](const std::vector<uint8_t>& data) {
    std::string text(data.begin(), data.end());
    logArea->append(text);
});
```

### Styled Log with Dark Theme

```cpp
TextArea* log = new TextArea(20, 80, 560, 300);
window->add(log);

log->setFont(L"Consolas", 11, false);
log->setTextColor(RGB(170, 180, 195));
log->setBackColor(RGB(22, 22, 28));

log->append(L"Application started\r\n");
log->append(L"> Waiting for data...\r\n");
```

### Display with Auto-scroll

```cpp
TextArea* ta = new TextArea(10, 10, 400, 200);
window->add(ta);

ta->append("Line 1\r\n");
ta->append("Line 2\r\n");
ta->append("Line 3\r\n");  // auto-scrolls to bottom
```

### Setting Full Text

```cpp
ta->setText("New content\r\nSecond line");
ta->clear();  // Clears everything
```

## Notes

- **Style:** `ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY`
- Newlines: use `\r\n` (Windows convention)
- `append()` auto-scrolls to bottom (`SB_BOTTOM`)
- Encoding: internally Unicode (`std::wstring`). Methods with `const char*` / `std::string` convert from UTF-8
- IDs start at **4000**
- Font and color styling uses the same mechanism as `Label` — colors handled by `WM_CTLCOLORSTATIC` in `SimpleWindow`
