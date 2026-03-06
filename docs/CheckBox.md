# CheckBox — Checkbox

> `#include <UI/CheckBox/CheckBox.h>`

## Description

Checkbox with `onChange` callback. State toggles automatically on click (`BS_AUTOCHECKBOX` style).

## Constructor

```cpp
CheckBox(int x, int y, int width, int height, const char* text,
         bool checked = false,
         std::function<void(CheckBox*, bool)> onChange = nullptr);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size (including label text) |
| `text` | `const char*` | Label text (UTF-8) |
| `checked` | `bool` | Initial state |
| `onChange` | `function<void(CheckBox*, bool)>` | Callback with state |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the control |
| `setText(const char* text)` | `void` | Changes text |
| `getText()` | `std::string` | Gets text |
| `setChecked(bool checked)` | `void` | Sets state programmatically |
| `isChecked()` | `bool` | Checks state |
| `handleClick()` | `void` | Click handler |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 3000) |

## Examples

### Simple Checkbox

```cpp
window->add(new CheckBox(20, 50, 200, 25, "Enable notifications", true,
    [](CheckBox* cb, bool checked) {
        if (checked) {
            // Enabled
        } else {
            // Disabled
        }
    }
));
```

### Programmatic Control

```cpp
CheckBox* cbAuto = new CheckBox(20, 50, 200, 25, "Auto mode", false);
window->add(cbAuto);

// Later:
cbAuto->setChecked(true);          // Check
bool state = cbAuto->isChecked();  // Read state
```

## Notes

- IDs start at **3000**
- Style: `BS_AUTOCHECKBOX` — Windows automatically toggles the state
- `isChecked()` reads the actual control state via `BM_GETCHECK`
