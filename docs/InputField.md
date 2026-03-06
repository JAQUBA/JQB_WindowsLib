# InputField — Edit Field

> `#include <UI/InputField/InputField.h>`

## Description

Single-line edit field with support for:
- Placeholder text
- Character limit
- Readonly mode
- Password mode
- `onTextChange` callback

## Constructor

```cpp
InputField(int x, int y, int width, int height,
           const char* defaultText = "",
           std::function<void(InputField*, const char*)> onTextChange = nullptr);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size |
| `defaultText` | `const char*` | Initial text (UTF-8) |
| `onTextChange` | `function<void(InputField*, const char*)>` | Callback on text change or Enter |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the EDIT control |
| `setText(const char* text)` | `void` | Sets text (UTF-8) |
| `getText()` | `std::string` | Gets text (UTF-8) |
| `setPlaceholder(const char* text)` | `void` | Placeholder text (`EM_SETCUEBANNER`) |
| `setMaxLength(int maxLength)` | `void` | Character limit |
| `setReadOnly(bool readOnly)` | `void` | Readonly mode |
| `setPassword(bool isPassword)` | `void` | Password mode (character masking) |
| `handleTextChange()` | `void` | Invokes callback |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 2000) |

## Examples

### Field with Placeholder

```cpp
InputField* input = new InputField(20, 50, 200, 25, "");
input->setPlaceholder("Enter name...");
window->add(input);
```

### Field with Callback

```cpp
InputField* search = new InputField(20, 50, 200, 25, "",
    [](InputField* field, const char* text) {
        // React to text change or Enter
    }
);
window->add(search);
```

### Password Field with Limit

```cpp
InputField* password = new InputField(20, 100, 200, 25, "");
password->setPassword(true);
password->setMaxLength(32);
password->setPlaceholder("Password...");
window->add(password);
```

### Reading Value

```cpp
std::string value = inputField->getText();
```

## Notes

- IDs start at **2000**
- Style: `WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL`
- Subclassing captures `WM_CHAR` (Enter) and `WM_COMMAND` (EN_CHANGE)
- Placeholder requires Common Controls 6.0 (available by default in Windows Vista+)
