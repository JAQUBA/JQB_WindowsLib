# TabControl — Tab Control

> `#include <UI/TabControl/TabControl.h>`

## Description

Tab control with panels. Each tab has a separate panel (`HWND`) on which other controls can be placed. Supports:
- Adding/removing tabs
- Callback on tab change
- Automatic hiding/showing of panels

## Constructor

```cpp
TabControl(int x, int y, int width, int height);
```

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the `WC_TABCONTROL` control |
| `addTab(const char* title)` | `int` | Adds a tab, returns index |
| `getTabPage(int index)` | `HWND` | Tab panel handle (for placing controls) |
| `removeTab(int index)` | `void` | Removes a tab |
| `selectTab(int index)` | `void` | Programmatic tab selection |
| `getSelectedTab()` | `int` | Active tab index |
| `setTabTitle(int index, const char* title)` | `void` | Changes tab title |
| `getTabTitle(int index)` | `std::string` | Gets title |
| `getTabCount()` | `int` | Number of tabs |
| `onTabChange(function<void(int)> cb)` | `void` | Tab change callback |
| `handleSelection()` | `void` | Internal `TCN_SELCHANGE` handler |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 7000) |

## Examples

### Tabs with Panels

```cpp
TabControl* tabs = new TabControl(10, 10, 580, 350);
window->add(tabs);

int tab1 = tabs->addTab("Settings");
int tab2 = tabs->addTab("Log");
int tab3 = tabs->addTab("About");

// Get panel handle and add controls
HWND page1 = tabs->getTabPage(tab1);

// Creating controls on the tab panel
CreateWindowW(L"BUTTON", L"Button on Tab 1",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    20, 20, 120, 30,
    page1, NULL, _core.hInstance, NULL);
```

### Tab Change Callback

```cpp
tabs->onTabChange([](int selectedIndex) {
    // selectedIndex — index of the newly selected tab
    switch (selectedIndex) {
        case 0: /* Tab 1 */ break;
        case 1: /* Tab 2 */ break;
        case 2: /* Tab 3 */ break;
    }
});
```

### Programmatic Tab Switch

```cpp
tabs->selectTab(1);  // Switch to second tab
```

### Dynamic Management

```cpp
tabs->setTabTitle(0, "New Title");
tabs->removeTab(2);
int count = tabs->getTabCount();
```

## Panel Architecture

Each tab has a separate panel (a `STATIC` control with white background), which is a child of TabControl:

```
TabControl (WC_TABCONTROL)
├── Tab 0: "Settings"   →  Panel HWND  (STATIC, SS_WHITERECT)
│                            ├── Button
│                            └── Label
├── Tab 1: "Log"         →  Panel HWND
│                            └── TextArea
└── Tab 2: "About"       →  Panel HWND
                             └── Label
```

Only the active panel is visible (`SW_SHOW`), the rest are hidden (`SW_HIDE`).

## Notes

- IDs start at **7000**
- Tab panels are `STATIC` controls with `SS_WHITERECT`
- `handleSelection()` is called by `SimpleWindow::WindowProc` on `WM_NOTIFY` + `TCN_SELCHANGE`
- To place JQB components on a tab panel, use `getTabPage()` as the parent in `create()`
- Titles support Unicode characters (UTF-8 → UTF-16, with ANSI fallback)
