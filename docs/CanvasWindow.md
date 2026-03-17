# CanvasWindow

Reusable zoomable/pannable GDI canvas — a child window for 2D vector rendering with double-buffered painting.

## Features

- Mouse wheel zoom (towards cursor position)
- Left-button drag to pan
- Double-click to reset view
- Optional grid overlay with configurable spacing and extent
- Double-buffered GDI painting (no flicker)
- World-to-screen coordinate transforms (mm → pixels)
- Configurable default zoom and pan offset
- Y-axis flipped (positive Y = up, matching standard math/engineering convention)

## Include

```cpp
#include <UI/CanvasWindow/CanvasWindow.h>
```

## Usage

### Subclassing

Override `onDraw()` to render custom content:

```cpp
class MyCanvas : public CanvasWindow {
protected:
    void onDraw(HDC hdc, const RECT& rc) override {
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(0, 200, 100));
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        MoveToEx(hdc, (int)toScreenX(0), (int)toScreenY(0), NULL);
        LineTo(hdc, (int)toScreenX(100), (int)toScreenY(50));

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
};
```

### Creating and Configuring

```cpp
MyCanvas* canvas = new MyCanvas();
canvas->create(parentHwnd, 0, 60, 800, 540);

// Grid
canvas->setGridVisible(true);
canvas->setGridSpacing(10.0);          // 10mm grid
canvas->setGridExtent(500.0, 500.0);   // grid covers 500×500mm

// Colors
canvas->setBackgroundColor(RGB(30, 30, 38));
canvas->setGridColor(RGB(50, 50, 60));

// Default view
canvas->setDefaultZoom(2.0);
canvas->setDefaultPan(50.0, 50.0);

canvas->redraw();
```

## API Reference

### Construction / Destruction

| Method | Description |
|--------|-------------|
| `CanvasWindow()` | Constructor — initializes default state |
| `virtual ~CanvasWindow()` | Destructor — destroys the window |

### Window Management

| Method | Description |
|--------|-------------|
| `void create(HWND parent, int x, int y, int w, int h)` | Create the child window |
| `HWND getHandle() const` | Get the underlying HWND |
| `void redraw()` | Request a repaint (invalidates the window) |
| `void resetView()` | Reset zoom and pan to defaults |

### Grid Configuration

| Method | Description |
|--------|-------------|
| `void setGridVisible(bool visible)` | Show/hide the grid overlay |
| `void setGridSpacing(double spacing)` | Set grid line spacing in world units (mm) |
| `void setGridExtent(double w, double h)` | Set grid area size in world units |
| `void setGridColor(COLORREF color)` | Set grid line color |

### Appearance

| Method | Description |
|--------|-------------|
| `void setBackgroundColor(COLORREF color)` | Set canvas background color |

### View Defaults

| Method | Description |
|--------|-------------|
| `void setDefaultZoom(double zoom)` | Set default zoom level (used on reset) |
| `void setDefaultPan(double x, double y)` | Set default pan offset (used on reset) |

### Coordinate Helpers

| Method | Description |
|--------|-------------|
| `double toScreenX(double worldX)` | Convert world X coordinate to screen pixel |
| `double toScreenY(double worldY)` | Convert world Y coordinate to screen pixel (Y-flipped) |

### Virtual Methods (Override in Subclass)

| Method | Description |
|--------|-------------|
| `virtual void onDraw(HDC hdc, const RECT& clientRect)` | Called during paint — draw custom content here |

## Mouse Interaction

| Action | Effect |
|--------|--------|
| Mouse wheel | Zoom towards cursor position |
| Left-button drag | Pan the view |
| Double-click | Reset to default zoom/pan |

## Notes

- Not a `UIComponent` — manages its own HWND lifecycle. Delete manually when done.
- Uses `GWLP_USERDATA` pattern for WndProc routing.
- World coordinates are in millimeters.
- Y-axis is flipped: positive Y goes up on screen.
- Window class name: `L"JQB_CanvasWindow"`.
