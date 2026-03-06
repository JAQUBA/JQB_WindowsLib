# Label — Text Label

> `#include <UI/Label/Label.h>`

## Description

Static text label. Displays text in Unicode format (UTF-16, `wchar_t*`). Ideal for displaying statuses, descriptions, and headers. Supports font and color styling.

## Constructor

```cpp
Label(int x, int y, int width, int height, const wchar_t* text);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Top-left corner position |
| `width`, `height` | `int` | Label size |
| `text` | `const wchar_t*` | Label text (Unicode, use `L` prefix) |

> **Note:** Label takes `wchar_t*` (not `char*` like other components). Use the `L` prefix:

```cpp
new Label(10, 10, 200, 20, L"Hello World!");
```

## Methods

### Text

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the `STATIC` control |
| `setText(const wchar_t* text)` | `void` | Changes label text |
| `getText()` | `const wchar_t*` | Gets current text |
| `getX()`, `getY()` | `int` | Position |
| `getWidth()`, `getHeight()` | `int` | Size |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 2000) |

### Font and Color Styling

| Method | Returns | Description |
|--------|---------|-------------|
| `setFont(const wchar_t* fontName, int size, bool bold = false, bool italic = false)` | `void` | Sets font |
| `setTextColor(COLORREF color)` | `void` | Sets text color |
| `setBackColor(COLORREF color)` | `void` | Sets background color |

> Colors are handled by `WM_CTLCOLORSTATIC` in `SimpleWindow`. The label must be added to the window before setting colors.

## Examples

### Basic Label

```cpp
Label* lbl = new Label(20, 10, 300, 25, L"Status: Waiting...");
window->add(lbl);
```

### Styled Label

```cpp
Label* lbl = new Label(20, 10, 300, 25, L"Measurement");
window->add(lbl);

lbl->setFont(L"Segoe UI", 14, true);
lbl->setTextColor(RGB(0, 220, 80));
lbl->setBackColor(RGB(40, 40, 50));
```

### Dynamic Text Update

```cpp
Label* lblResult = new Label(20, 10, 300, 25, L"");
window->add(lblResult);

// Later in the program:
lblResult->setText(L"Connected to device!");
lblResult->setText(L"Value: 123.45 V");
```

### UTF-8 → wchar_t Conversion

```cpp
#include <Util/StringUtils.h>

std::string utf8Text = "Voltage: 12.5V";
std::wstring wideText = StringUtils::utf8ToWide(utf8Text);
label->setText(wideText.c_str());
```

## Notes

- Style: `WS_CHILD | WS_VISIBLE | SS_LEFT` — left-aligned text
- IDs start at **2000**
- Label does not handle click events
