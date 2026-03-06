# Button — Button

> `#include <UI/Button/Button.h>`

## Description

Button with support for:
- **Click** (`onClick`) — callback invoked on short press
- **Long press** (`onLongClick`) — callback after holding ≥ 800 ms

## Constructor

```cpp
Button(int x, int y, int width, int height, const char* text,
       std::function<void(Button*)> onClick,
       std::function<void(Button*)> onLongClick = nullptr);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Top-left position (pixels, relative to parent) |
| `width`, `height` | `int` | Button size |
| `text` | `const char*` | Button text (UTF-8) |
| `onClick` | `function<void(Button*)>` | Click callback |
| `onLongClick` | `function<void(Button*)>` | Long press callback (optional) |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the Windows control (called automatically by `SimpleWindow::add()`) |
| `handleClick()` | `void` | Invokes `onClick` callback |
| `handleLongClick()` | `void` | Invokes `onLongClick` callback |
| `setBackColor(COLORREF color)` | `void` | Background color |
| `setTextColor(COLORREF color)` | `void` | Text color |
| `setHoverColor(COLORREF color)` | `void` | Hover background color |
| `getX()` | `int` | X position |
| `getY()` | `int` | Y position |
| `getWidth()` | `int` | Width |
| `getHeight()` | `int` | Height |
| `getText()` | `const char*` | Button text |
| `getHandle()` | `HWND` | Windows control handle |
| `getId()` | `int` | Unique ID (auto-generated from 1000) |

## Long Press Mechanism

- Button captures `WM_LBUTTONDOWN` / `WM_LBUTTONUP` via subclassing
- Timer checks press duration every 50 ms
- If ≥ 800 ms → `onLongClick` is called and `onClick` is **not** invoked
- If < 800 ms → only `onClick`

## Examples

### Button with Click

```cpp
window->add(new Button(20, 20, 120, 35, "Click",
    [](Button* btn) {
        // Handle click
    }
));
```

### Button with Long Press

```cpp
window->add(new Button(20, 60, 120, 35, "Hold",
    [](Button* btn) {
        // Short click
    },
    [](Button* btn) {
        // Long press (≥ 800 ms)
    }
));
```

### Styled Button

```cpp
Button* btn = new Button(20, 20, 120, 35, "Connect", [](Button*) { /* ... */ });
btn->setBackColor(RGB(40, 130, 200));
btn->setTextColor(RGB(255, 255, 255));
btn->setHoverColor(RGB(50, 150, 220));
window->add(btn);
```

### Button with Access to Other Components

```cpp
Label* status = new Label(20, 20, 200, 25, L"Ready");
window->add(status);

window->add(new Button(20, 60, 100, 30, "Action",
    [status](Button* btn) {
        status->setText(L"Action performed!");
    }
));
```

## Notes

- Button IDs start at **1000** (auto-increment)
- Button text is converted from UTF-8 to UTF-16 automatically
- Destructor automatically cleans up internal maps and destroys the window
