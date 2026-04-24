# LogWindow

Standalone log window (separate top-level window) with optional persistent position.

## Header

```cpp
#include <UI/LogWindow/LogWindow.h>
```

## What It Is For

Use `LogWindow` when logs should be visible independently from your main UI.
This is useful in tools that have dense dashboards, editors, or canvases.

## Main API

- `setTitle(const wchar_t* title)`
- `setTextColor(COLORREF color)`
- `setBackColor(COLORREF color)`
- `setFont(const wchar_t* fontName, int size)`
- `enablePersistence(ConfigManager& config, const std::string& prefix = "logwin")`
- `bool open(HWND parentHwnd = NULL)`
- `void close()`
- `bool isOpen() const`
- `HWND getHandle() const`
- `void appendMessage(const wchar_t* msg)`
- `void clear()`

## Minimal Example

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/LogWindow/LogWindow.h>
#include <Util/ConfigManager.h>

SimpleWindow* window = nullptr;
LogWindow* logWindow = nullptr;
ConfigManager config("app.ini");

void setup() {
    window = new SimpleWindow(520, 240, "LogWindow Example", 0);
    window->init();

    logWindow = new LogWindow();
    logWindow->setTitle(L"Application Log");
    logWindow->setFont(L"Consolas", 13);
    logWindow->setTextColor(RGB(170, 180, 195));
    logWindow->setBackColor(RGB(22, 22, 28));
    logWindow->enablePersistence(config, "logwin");

    window->add(new Button(20, 20, 160, 34, "Open Log", [](Button*) {
        logWindow->open(window->getHandle());
        logWindow->appendMessage(L"Log opened");
    }));

    window->add(new Button(20, 64, 160, 34, "Write Message", [](Button*) {
        if (logWindow->isOpen())
            logWindow->appendMessage(L"Background task completed");
    }));

    window->add(new Button(20, 108, 160, 34, "Clear Log", [](Button*) {
        if (logWindow->isOpen())
            logWindow->clear();
    }));

    window->onClose([]() {
        if (logWindow) {
            logWindow->close();
            delete logWindow;
            logWindow = nullptr;
        }
    });
}

void loop() {}
```

## Persistence Keys

With prefix `logwin`, these keys are stored in config:

- `logwin_x`
- `logwin_y`
- `logwin_w`
- `logwin_h`

## Notes

1. Configure title/font/colors before calling `open()`.
2. `appendMessage()` appends a line and auto-scrolls.
3. `close()` persists position if persistence is enabled.
4. `LogWindow` is not a `UIComponent`; manage its lifetime manually.
