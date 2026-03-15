# SimpleWindow — Main Application Window

> `#include <UI/SimpleWindow/SimpleWindow.h>`

## Description

`SimpleWindow` is the main application window. It manages window creation, the message loop, and a collection of UI components. Handles click events, selection changes, control notifications, and a timer for button long-press detection.

## Constructor

```cpp
SimpleWindow(int width, int height, const char* title, int iconId);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `width` | `int` | Window width in pixels (client area) |
| `height` | `int` | Window height in pixels (client area) |
| `title` | `const char*` | Window title (UTF-8) |
| `iconId` | `int` | Icon ID from resources (`resources/resources.rc`). Pass `0` for no icon. |

## Methods

### `bool init()`

Registers the window class, creates the window, and shows it. **Must be called after the constructor.**

```cpp
SimpleWindow* win = new SimpleWindow(800, 600, "My App", 1);
if (!win->init()) {
    // Window creation error
}
```

**Returns:** `true` if the window was created, `false` on error.

### `void add(UIComponent* component)`

Adds any UI component to the window. The component is automatically created (`create()`) with the window handle as parent.

```cpp
Button* btn = new Button(10, 10, 100, 30, "OK", [](Button*){});
win->add(btn);
```

### `add()` Overloads (backward compatibility)

```cpp
void add(Button* button);
void add(Label* label);
void add(Select* select);
void add(TextArea* textArea);
void add(ValueDisplay* valueDisplay);
void add(Chart* chart);
```

### `void close()`

Closes the window — sends `WM_CLOSE`.

```cpp
win->close();
```

### Menu Support

```cpp
void setMenu(HMENU menu);                              // Set menu bar
void onMenuCommand(std::function<void(int)> callback);  // Menu command handler
```

### Background Color

```cpp
void setBackgroundColor(COLORREF color);  // Changes window background (WM_ERASEBKGND)
```

## Memory Management

`SimpleWindow` **owns** all added components. The destructor automatically frees:
- All components from the `m_components` vector (`delete`)
- The Windows window (`DestroyWindow`)
- Sends `PostQuitMessage(0)` — ends the message loop

> **Important:** Do not manually delete components added to the window!

## Event Handling (internals)

`WindowProc` automatically handles:

| Message | Handling |
|---------|----------|
| `WM_COMMAND` + `BN_CLICKED` | `component->handleClick()` |
| `WM_COMMAND` + `BN_PUSHED` | `startButtonPress()` → long press |
| `WM_COMMAND` + `CBN_SELCHANGE` | `component->handleSelection()` |
| `WM_NOTIFY` + `TCN_SELCHANGE` | `component->handleSelection()` (TabControl) |
| `WM_CTLCOLORSTATIC` | Custom colors for Label and TextArea |
| `WM_TIMER` | `checkForLongPresses()` |
| `WM_CLOSE` | `DestroyWindow()` |
| `WM_DESTROY` | `PostQuitMessage(0)` |

## Example

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/Select/Select.h>

SimpleWindow* window;

void setup() {
    window = new SimpleWindow(500, 350, "Demo Window", 0);
    window->init();

    // Background color
    window->setBackgroundColor(RGB(30, 30, 38));

    // Label
    window->add(new Label(20, 20, 200, 25, L"Choose an option:"));

    // Combo box
    Select* sel = new Select(20, 50, 200, 25, "Option 1", [](Select* s) {
        // react to selection change
    });
    sel->addItem("Option 1");
    sel->addItem("Option 2");
    sel->addItem("Option 3");
    window->add(sel);

    // Close button
    window->add(new Button(20, 100, 120, 35, "Close", [](Button*) {
        window->close();
    }));
}

void loop() {}
```

## Notes

- Window uses `WS_OVERLAPPEDWINDOW` style (with minimize, maximize, close buttons).
- The size given in the constructor is the **client area** size. The actual window size is larger by the frame and title bar (`AdjustWindowRect`).
- Currently only **one** `SimpleWindow` instance is supported (singleton via `s_instance`). For additional windows use `OverlayWindow` or raw WinAPI with `GWLP_USERDATA`.
