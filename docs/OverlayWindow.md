# OverlayWindow — Overlay Window (e.g. OBS)

> `#include <UI/OverlayWindow/OverlayWindow.h>`

## Description

Base class for overlay windows — separate WinAPI windows (not `SimpleWindow`!) with:
- `WS_EX_TOOLWINDOW` — no icon on taskbar
- Always-on-top (optional)
- Double-buffered GDI rendering (no flicker)
- Built-in context menu (chroma key colors, pin, close)
- Position and style persistence via `ConfigManager`

Subclass overrides `onPaint()` to render custom content.

## Constructor

```cpp
OverlayWindow(const wchar_t* className, const wchar_t* title,
              int defaultW = 420, int defaultH = 160);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `className` | `const wchar_t*` | WinAPI window class name (must be unique) |
| `title` | `const wchar_t*` | Window title |
| `defaultW` | `int` | Default width (pixels) |
| `defaultH` | `int` | Default height (pixels) |

## Methods

### Window Lifecycle

| Method | Returns | Description |
|--------|---------|-------------|
| `open(HWND parentHwnd = NULL)` | `bool` | Creates and shows the window |
| `close()` | `void` | Destroys the window |
| `isOpen()` | `bool` | Whether the window is currently open |
| `getHandle()` | `HWND` | Window handle |

### Appearance

| Method | Returns | Description |
|--------|---------|-------------|
| `setAlwaysOnTop(bool onTop)` | `void` | Toggle always-on-top |
| `isAlwaysOnTop()` | `bool` | Current always-on-top state |
| `setBackgroundColor(COLORREF color)` | `void` | Background color |
| `setTextColor(COLORREF color)` | `void` | Text color |
| `getBackgroundColor()` | `COLORREF` | Current background color |
| `getTextColor()` | `COLORREF` | Current text color |
| `setMinSize(int minW, int minH)` | `void` | Minimum window size (default: 200×80) |

### Persistence

| Method | Description |
|--------|-------------|
| `enablePersistence(ConfigManager& config, const std::string& prefix)` | Auto-save/load position, size, colors, and always-on-top to INI file |

Saved keys (with prefix, e.g. `overlay_`):
- `{prefix}_x`, `{prefix}_y` — position
- `{prefix}_w`, `{prefix}_h` — size
- `{prefix}_ontop` — always-on-top flag
- `{prefix}_bg` — background color (COLORREF)
- `{prefix}_text` — text color (COLORREF)

## Virtual Methods (for subclasses)

### `onPaint(HDC memDC, const RECT& clientRect)` — **pure virtual**

Called during `WM_PAINT` with a double-buffered memory DC. Background is already filled — only draw content.

```cpp
void onPaint(HDC memDC, const RECT& rc) override {
    HFONT font = CreateFontW(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Consolas");
    HFONT oldFont = (HFONT)SelectObject(memDC, font);
    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, m_textColor);
    DrawTextW(memDC, L"12.345 V", -1, (LPRECT)&rc,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(memDC, oldFont);
    DeleteObject(font);
}
```

### `onBuildContextMenu(HMENU menu)`

Called before showing the context menu. Add custom items here.

```cpp
void onBuildContextMenu(HMENU menu) override {
    AppendMenuW(menu, MF_STRING, 9150, L"My Custom Action");
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
}
```

### `onMenuCommand(int cmdId)` → `bool`

Handle custom context menu commands. Return `true` if handled.

```cpp
bool onMenuCommand(int cmdId) override {
    if (cmdId == 9150) {
        // Handle custom action
        return true;
    }
    return false;
}
```

## Protected Members

| Member | Type | Description |
|--------|------|-------------|
| `m_bgColor` | `COLORREF` | Background color (accessible in subclass) |
| `m_textColor` | `COLORREF` | Text color (accessible in subclass) |

## Helper Method

| Method | Description |
|--------|-------------|
| `invalidate()` | Request repaint (call after data changes) |

## Context Menu IDs

| Range | Owner |
|-------|-------|
| 9100–9149 | Base class (always-on-top, colors, close) |
| 9150+ | Subclass custom items |

Built-in context menu items:
- **Always on top** — toggle with checkmark
- **Background** — Black, Green, Blue, Magenta (chroma key presets)
- **Text color** — Green, White, Yellow, Cyan
- **Close** — closes the overlay window

## Examples

### Minimal Subclass

```cpp
class MyOverlay : public OverlayWindow {
public:
    MyOverlay()
        : OverlayWindow(L"MyOverlayClass", L"My Overlay", 420, 160) {}

protected:
    void onPaint(HDC memDC, const RECT& rc) override {
        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, m_textColor);
        DrawTextW(memDC, L"Hello!", -1, (LPRECT)&rc,
                  DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
};
```

### With Persistence and Custom Data

```cpp
class MeterOverlay : public OverlayWindow {
public:
    MeterOverlay()
        : OverlayWindow(L"MeterOverlay", L"Meter Overlay", 420, 160) {
        enablePersistence(config, "overlay");
    }

    void updateValue(const std::wstring& val) {
        m_value = val;
        invalidate();
    }

protected:
    void onPaint(HDC memDC, const RECT& rc) override {
        // Render m_value with custom fonts...
    }

private:
    std::wstring m_value;
};

// Usage
auto* overlay = new MeterOverlay();
overlay->open();
overlay->setBackgroundColor(RGB(0, 0, 0));
overlay->setTextColor(RGB(0, 255, 0));
overlay->updateValue(L"12.345 V");
```

### Extending Context Menu

```cpp
#define IDM_MY_RESET 9150

class MyOverlay : public OverlayWindow {
    // ...
protected:
    void onBuildContextMenu(HMENU menu) override {
        AppendMenuW(menu, MF_STRING, IDM_MY_RESET, L"Reset Display");
        AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    }

    bool onMenuCommand(int cmdId) override {
        if (cmdId == IDM_MY_RESET) {
            m_value = L"---";
            invalidate();
            return true;
        }
        return false;
    }
};
```

## Notes

- **Not a singleton** — you can create multiple overlay windows (unlike `SimpleWindow`)
- Window uses `GWLP_USERDATA` for `this` pointer routing in `WndProc`
- Double buffering: `CreateCompatibleDC` → `onPaint(memDC)` → `BitBlt` → delete
- `WM_ERASEBKGND` returns 1 (background drawn in `WM_PAINT`)
- Right-click opens the built-in context menu
- Resizable window with minimum size constraint (`WM_GETMINMAXINFO`)
- Position saved on `WM_MOVE` / `WM_SIZE` when persistence is enabled
