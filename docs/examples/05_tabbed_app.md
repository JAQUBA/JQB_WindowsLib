# Example 05 — Tabbed App

Application with tabs — each tab has its own panel with controls.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/TabControl/TabControl.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/InputField/InputField.h>
#include <UI/CheckBox/CheckBox.h>
#include <Util/StringUtils.h>

SimpleWindow* window;
TabControl*   tabs;

// Helper function to create controls directly on a tab panel
// (controls on tabs are NOT managed by SimpleWindow — use WinAPI)
HWND createStaticLabel(HWND parent, int x, int y, int w, int h, const wchar_t* text) {
    return CreateWindowW(L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, h, parent, NULL, _core.hInstance, NULL);
}

HWND createButton(HWND parent, int x, int y, int w, int h, const wchar_t* text, int id) {
    return CreateWindowW(L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, _core.hInstance, NULL);
}

void setup() {
    window = new SimpleWindow(600, 400, "Tabbed Application", 0);
    window->init();

    // --- TabControl ---
    tabs = new TabControl(10, 10, 570, 340);
    window->add(tabs);

    // Add tabs
    tabs->addTab("Home");
    tabs->addTab("Settings");
    tabs->addTab("About");

    // --- Tab 0: Home ---
    HWND page0 = tabs->getTabPage(0);
    createStaticLabel(page0, 20, 20, 400, 25, L"Welcome to the main page!");
    createStaticLabel(page0, 20, 55, 400, 25, L"Select a tab above.");

    // --- Tab 1: Settings ---
    HWND page1 = tabs->getTabPage(1);
    createStaticLabel(page1, 20, 20, 150, 22, L"Device name:");
    CreateWindowW(L"EDIT", L"MyDevice",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        180, 18, 250, 24, page1, NULL, _core.hInstance, NULL);

    createStaticLabel(page1, 20, 55, 150, 22, L"Mode:");
    HWND combo = CreateWindowW(L"COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        180, 53, 250, 200, page1, NULL, _core.hInstance, NULL);
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Automatic");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Manual");
    SendMessageW(combo, CB_SETCURSEL, 0, 0);

    HWND chk = CreateWindowW(L"BUTTON", L"Enable notifications",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 90, 300, 22, page1, NULL, _core.hInstance, NULL);
    SendMessage(chk, BM_SETCHECK, BST_CHECKED, 0);

    createButton(page1, 20, 130, 120, 30, L"Save", 9001);

    // --- Tab 2: About ---
    HWND page2 = tabs->getTabPage(2);
    createStaticLabel(page2, 20, 20, 500, 25, L"Tabbed App v1.0");
    createStaticLabel(page2, 20, 50, 500, 25, L"Built with JQB_WindowsLib");
    createStaticLabel(page2, 20, 80, 500, 25, L"github.com/JAQUBA/JQB_WindowsLib");

    // Tab change callback (optional)
    tabs->onTabChange([](int newIndex) {
        // React to tab change
    });
}

void loop() {}
```

## Key Points

1. **TabControl** — creates tabs with separate HWND panels
   - `addTab("Name")` — adds a tab
   - `getTabPage(index)` — returns the tab panel HWND
   - `onTabChange(callback)` — callback on tab change

2. **Controls on tabs** — placed directly on the panel HWND
   - Use `CreateWindowW()` from WinAPI, passing `tabs->getTabPage(i)` as parent
   - These controls are **not** managed by `SimpleWindow::add()` — you manage them manually

3. **`_core.hInstance`** — global HINSTANCE needed for `CreateWindow`

4. **Alternative**: You can also create separate `SimpleWindow` dialogs instead of tabs — depends on UI complexity
