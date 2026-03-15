# ProgressBar — Progress Bar

> `#include <UI/ProgressBar/ProgressBar.h>`

## Description

Native Windows progress bar supporting standard mode (0–100%) and Marquee mode (indeterminate progress animation). Custom colors are supported with automatic visual styles bypass.

## Constructor

```cpp
ProgressBar(int x, int y, int width, int height,
            int minimum = 0, int maximum = 100);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size |
| `minimum` | `int` | Minimum value (default 0) |
| `maximum` | `int` | Maximum value (default 100) |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the `PROGRESS_CLASS` control |
| `setProgress(int value)` | `void` | Sets value (clamped to range) |
| `getProgress()` | `int` | Gets current value |
| `setRange(int min, int max)` | `void` | Changes range |
| `getRange(int& min, int& max)` | `void` | Gets range |
| `setColor(COLORREF color)` | `void` | Bar color |
| `setBackColor(COLORREF color)` | `void` | Background color |
| `setMarquee(bool enable, int interval)` | `void` | Marquee mode (animation) |
| `step(int increment)` | `void` | Increments value |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 6000) |

## Examples

### Percentage Bar

```cpp
ProgressBar* pb = new ProgressBar(20, 50, 300, 25, 0, 100);
window->add(pb);

pb->setProgress(50);   // 50%
pb->step(10);          // 60%
pb->setColor(RGB(0, 200, 0));  // green
```

### Marquee Mode (Indeterminate Progress)

```cpp
ProgressBar* pb = new ProgressBar(20, 50, 300, 25);
window->add(pb);
pb->setMarquee(true, 30);   // Animation every 30 ms

// After operation completes:
pb->setMarquee(false);
pb->setProgress(100);
```

### Custom Colors

```cpp
ProgressBar* pb = new ProgressBar(20, 50, 300, 25);
window->add(pb);
pb->setColor(RGB(40, 130, 200));     // blue bar
pb->setBackColor(RGB(30, 30, 38));   // dark background
pb->setProgress(75);
```

### Range Change

```cpp
pb->setRange(0, 1000);
pb->setProgress(500);   // 50%
```

## Notes

- IDs start at **6000**
- `setColor()` / `setBackColor()` use `PBM_SETBARCOLOR` / `PBM_SETBKCOLOR`
- **Visual styles bypass:** When custom colors are set, visual styles are automatically disabled on the control via `uxtheme.dll` → `SetWindowTheme(hwnd, L"", L"")`. This is required because Common Controls v6 (enabled by `resources/app.manifest`) ignore `PBM_SETBARCOLOR` / `PBM_SETBKCOLOR` when visual styles are active.
- Marquee mode adds `PBS_MARQUEE` style to the control
- `step(n)` uses `PBM_DELTAPOS` — adds `n` to current value
