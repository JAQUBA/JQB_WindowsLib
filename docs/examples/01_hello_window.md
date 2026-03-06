# Example 01 — Hello Window

Minimal application with a window, a label, and a button.

## `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
```

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>

SimpleWindow* window;
Label* lblInfo;
int clickCount = 0;

void setup() {
    window = new SimpleWindow(450, 250, "Hello JQB_WindowsLib", 0);
    window->init();

    window->add(new Label(20, 20, 410, 30, L"Welcome! This is your first application."));

    lblInfo = new Label(20, 60, 410, 25, L"Clicks: 0");
    window->add(lblInfo);

    window->add(new Button(20, 100, 180, 35, "Click me!", [](Button* btn) {
        clickCount++;
        wchar_t buf[64];
        swprintf(buf, 64, L"Clicks: %d", clickCount);
        lblInfo->setText(buf);
    }));

    window->add(new Button(220, 100, 180, 35, "Reset", [](Button* btn) {
        clickCount = 0;
        lblInfo->setText(L"Clicks: 0");
    }));
}

void loop() {
    // Empty — the application is fully event-driven
}
```

## Key Points

1. **SimpleWindow** creates the main window — parameters: width, height, title (UTF-8), icon resource ID (0 = none)
2. **Always call `window->init()`** after creating the window
3. **Label** accepts text as `const wchar_t*` (prefix `L`)
4. **Button** accepts text as `const char*` (UTF-8) and a callback `std::function<void(Button*)>`
5. Components added via `add()` are managed by the window — **do not delete them manually**
6. `swprintf` for wide-string formatting (instead of `printf`)
