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
| `addDataPoint(double value, const wstring& unit)` | `void` | Adds a single data point (timestamped at call time) |
| `addDataPoints(const double* values, int count, double totalDurationMs)` | `void` | Adds a batch of data points with evenly distributed timestamps |
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
    bool isNewSegment = false;                    // Line break before this point
};
```

The `isNewSegment` flag is automatically set on the first point of each `addDataPoints()` batch.
When rendering, the chart starts a new line segment (no connecting line from the previous point)
at every point where `isNewSegment == true`. This prevents diagonal artifacts between non-contiguous
data batches (e.g. audio buffers arriving at irregular intervals).

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

### Audio / Oscilloscope Mode (Batch)

```cpp
// For audio or buffer-based data, use addDataPoints() to properly
// distribute timestamps across the buffer duration:
double buffer[256];
int count;
if (engine.getOutputSnapshot(buffer, 256, count)) {
    double durationMs = 2048.0 / 44100.0 * 1000.0; // ~46.4 ms
    chart->addDataPoints(buffer, count, durationMs);
}
```

> **Important:** Do **not** use `addDataPoint()` in a loop for buffer data —
> all points would get the same timestamp and overlap on the X axis.
> Use `addDataPoints()` instead, which spreads timestamps evenly.

## Rendering

The chart uses its own window class (`ChartClass`) with double-buffered `WM_PAINT`:
- Memory DC + BitBlt (no flicker)
- Black background
- Grid (dotted lines)
- Axes with labels (Y values, X time — fractional format for windows ≤ 5s)
- Line connecting data points — **broken at batch boundaries** (`isNewSegment`)
- Dots at data points (hidden when > 200 points for performance)
- Title at top

## Notes

- IDs start at **5000**
- Data older than `timeWindow` is automatically removed
- `addDataPoint()` respects `refreshRate` — there can be more data than redraws
- Auto-scale adds 10% margin above/below extreme values
- For flat lines (min ≈ max) a margin is automatically added
