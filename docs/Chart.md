# Chart — Real-time Chart

> `#include <UI/Chart/Chart.h>`

## Description

Real-time line chart with:
- Automatic Y-axis scaling
- Configurable X-axis time window
- Grid, axes, and labels
- Refresh rate limiting
- Automatic removal of old data

## Constructor

```cpp
Chart(int x, int y, int width, int height, const char* title = "Measurements");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size |
| `title` | `const char*` | Chart title (displayed at top) |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the chart control |
| `addDataPoint(double value, const wstring& unit)` | `void` | Adds a data point |
| `clear()` | `void` | Clears data |
| `render(HDC hdc)` | `void` | Draws the chart (internal) |
| `setTimeWindow(int seconds)` | `void` | X-axis time window (default 30 s) |
| `setColors(COLORREF grid, axis, data)` | `void` | Chart colors |
| `setAutoScale(bool)` | `void` | Y-axis auto-scaling (default true) |
| `setYRange(double min, double max)` | `void` | Manual Y range |
| `setRefreshRate(int ms)` | `void` | Min. interval between redraws (default 100 ms) |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 5000) |

## `DataPoint` Structure

```cpp
struct DataPoint {
    double value;                                 // Value
    std::chrono::steady_clock::time_point timestamp; // Timestamp
    std::wstring unit;                            // Unit (e.g. L"V")
};
```

## Examples

### Voltage Chart

```cpp
Chart* chart = new Chart(20, 100, 560, 250, "Voltage [V]");
chart->setTimeWindow(60);              // Last 60 seconds
chart->setColors(
    RGB(80, 80, 80),      // grid
    RGB(200, 200, 200),   // axes
    RGB(0, 255, 0)        // data
);
window->add(chart);

// Adding data (e.g. in receive callback):
chart->addDataPoint(12.45, L"V");
chart->addDataPoint(12.52, L"V");
```

### Manual Y-axis Scaling

```cpp
chart->setAutoScale(false);
chart->setYRange(0.0, 25.0);  // Range 0–25
```

### Fast Refresh

```cpp
chart->setRefreshRate(50);  // Refresh max every 50 ms (20 FPS)
```

## Rendering

The chart uses its own window class (`ChartClass`) with `WM_PAINT`:
- Black background
- Grid (dotted lines)
- Axes with labels (Y values, X time in seconds)
- Line connecting data points
- Dots at data points
- Title at top

## Notes

- IDs start at **5000**
- Data older than `timeWindow` is automatically removed
- `addDataPoint()` respects `refreshRate` — there can be more data than redraws
- Auto-scale adds 10% margin above/below extreme values
- For flat lines (min ≈ max) a margin is automatically added
