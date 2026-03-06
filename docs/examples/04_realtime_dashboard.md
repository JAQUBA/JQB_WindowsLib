# Example 04 — Realtime Dashboard

Dashboard with a real-time chart, value display, and progress bar.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/Chart/Chart.h>
#include <UI/ValueDisplay/ValueDisplay.h>
#include <UI/ProgressBar/ProgressBar.h>

#include <cmath>
#include <cstdlib>

SimpleWindow*   window;
Chart*          chart;
ValueDisplay*   valueDisp;
ProgressBar*    progress;

bool running = false;
DWORD lastTick = 0;
double simValue = 0.0;

void setup() {
    window = new SimpleWindow(800, 550, "Realtime Dashboard", 0);
    window->init();

    // --- Title ---
    window->add(new Label(20, 10, 760, 28, L"Dashboard — real-time data simulation"));

    // --- ValueDisplay (LCD display) ---
    valueDisp = new ValueDisplay(20, 50, 250, 100);
    window->add(valueDisp);

    // Display configuration
    ValueDisplay::DisplayConfig cfg;
    cfg.backgroundColor = RGB(0, 0, 0);
    cfg.textColor = RGB(0, 255, 128);
    cfg.borderColor = RGB(60, 60, 60);
    cfg.fontSize = 36;
    cfg.fontName = L"Consolas";
    cfg.precision = 1;
    cfg.showUnit = true;
    valueDisp->setConfig(cfg);
    valueDisp->setMode(L"\u00B0C");
    valueDisp->updateValue(L"--.-");

    // --- ProgressBar ---
    window->add(new Label(290, 50, 100, 20, L"Fill:"));
    progress = new ProgressBar(290, 75, 480, 25);
    window->add(progress);
    progress->setProgress(0);

    // --- Chart ---
    chart = new Chart(20, 170, 750, 320, "Temperature");
    window->add(chart);
    chart->setTimeWindow(30);  // 30 seconds visible
    chart->setAutoScale(true);
    chart->setColors(
        RGB(50, 50, 50),    // grid
        RGB(180, 180, 180), // axes
        RGB(0, 200, 255)    // data
    );
    chart->setRefreshRate(50); // 20 FPS

    // --- Control buttons ---
    window->add(new Button(20, 505, 120, 30, "Start", [](Button* btn) {
        running = true;
        lastTick = GetTickCount();
    }));

    window->add(new Button(150, 505, 120, 30, "Stop", [](Button* btn) {
        running = false;
    }));

    window->add(new Button(280, 505, 120, 30, "Clear", [](Button* btn) {
        chart->clear();
        valueDisp->updateValue(L"--.-");
        progress->setProgress(0);
        simValue = 0.0;
    }));
}

void loop() {
    if (!running) return;

    DWORD now = GetTickCount();
    if (now - lastTick < 100) return; // update every 100ms
    lastTick = now;

    // Simulation: sine wave + noise
    static double t = 0.0;
    t += 0.1;
    simValue = 22.0 + 5.0 * sin(t) + ((rand() % 100) - 50) * 0.02;

    // Update chart
    chart->addDataPoint(simValue, L"\u00B0C");

    // Update display
    wchar_t buf[32];
    swprintf(buf, 32, L"%.1f", simValue);
    valueDisp->updateValue(buf);

    // Update progress bar (mapping 15-30°C → 0-100%)
    int pct = (int)((simValue - 15.0) / 15.0 * 100.0);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    progress->setProgress(pct);
}
```

## Key Points

1. **Chart** — real-time chart with auto-scale
   - `addDataPoint(value, unit)` — adds a data point with timestamp
   - `setTimeWindow(seconds)` — visible time window width
   - `setRefreshRate(ms)` — FPS limit (default 100ms = 10 FPS)
   - `setColors()` — grid, axes, and data color configuration
   - `clear()` — clears all data

2. **ValueDisplay** — LCD display with double-buffering
   - `DisplayConfig` struct → colors, font, precision, unit
   - `setMode(unit)` — sets the displayed unit
   - `updateValue(text)` — updates the value (no flicker thanks to double-buffer)

3. **ProgressBar** — progress bar 0-100%
   - `setProgress(percent)` — sets the value

4. **loop()** — ideal place for continuous data updates (polling with throttling)
