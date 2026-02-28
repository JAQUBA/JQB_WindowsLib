# Przykład 04 — Realtime Dashboard

Dashboard z wykresem czasu rzeczywistego, wyświetlaczem wartości i paskiem postępu.

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

    // --- Tytuł ---
    window->add(new Label(20, 10, 760, 28, L"Dashboard — symulacja danych w czasie rzeczywistym"));

    // --- ValueDisplay (wyświetlacz LCD) ---
    valueDisp = new ValueDisplay(20, 50, 250, 100);
    window->add(valueDisp);

    // Konfiguracja wyświetlacza
    ValueDisplay::DisplayConfig cfg;
    cfg.backgroundColor = RGB(0, 0, 0);
    cfg.textColor = RGB(0, 255, 128);
    cfg.borderColor = RGB(60, 60, 60);
    cfg.fontSize = 36;
    cfg.fontName = L"Consolas";
    cfg.precision = 1;
    cfg.showUnit = true;
    valueDisp->setConfig(cfg);
    valueDisp->setMode(L"°C");
    valueDisp->updateValue(L"--.-");

    // --- ProgressBar ---
    window->add(new Label(290, 50, 100, 20, L"Wypełnienie:"));
    progress = new ProgressBar(290, 75, 480, 25);
    window->add(progress);
    progress->setProgress(0);

    // --- Chart (wykres) ---
    chart = new Chart(20, 170, 750, 320, "Temperatura");
    window->add(chart);
    chart->setTimeWindow(30);  // 30 sekund widocznych
    chart->setAutoScale(true);
    chart->setColors(
        RGB(50, 50, 50),    // siatka
        RGB(180, 180, 180), // osie
        RGB(0, 200, 255)    // dane
    );
    chart->setRefreshRate(50); // 20 FPS

    // --- Przyciski sterujące ---
    window->add(new Button(20, 505, 120, 30, "Start", [](Button* btn) {
        running = true;
        lastTick = GetTickCount();
    }));

    window->add(new Button(150, 505, 120, 30, "Stop", [](Button* btn) {
        running = false;
    }));

    window->add(new Button(280, 505, 120, 30, "Wyczyść", [](Button* btn) {
        chart->clear();
        valueDisp->updateValue(L"--.-");
        progress->setProgress(0);
        simValue = 0.0;
    }));
}

void loop() {
    if (!running) return;

    DWORD now = GetTickCount();
    if (now - lastTick < 100) return; // aktualizacja co 100ms
    lastTick = now;

    // Symulacja: sinusoida + szum
    static double t = 0.0;
    t += 0.1;
    simValue = 22.0 + 5.0 * sin(t) + ((rand() % 100) - 50) * 0.02;

    // Aktualizuj wykres
    chart->addDataPoint(simValue, L"°C");

    // Aktualizuj wyświetlacz
    wchar_t buf[32];
    swprintf(buf, 32, L"%.1f", simValue);
    valueDisp->updateValue(buf);

    // Aktualizuj pasek postępu (mapowanie 15-30°C → 0-100%)
    int pct = (int)((simValue - 15.0) / 15.0 * 100.0);
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    progress->setProgress(pct);
}
```

## Kluczowe punkty

1. **Chart** — wykres czasu rzeczywistego z auto-scale
   - `addDataPoint(value, unit)` — dodaje punkt danych z timestampem
   - `setTimeWindow(seconds)` — szerokość okna czasowego
   - `setRefreshRate(ms)` — limit FPS (domyślnie 100ms = 10 FPS)
   - `setColors()` — konfiguracja kolorów siatki, osi, danych
   - `clear()` — czyści wszystkie dane

2. **ValueDisplay** — wyświetlacz LCD z double-bufferingiem
   - `DisplayConfig` struct → kolory, czcionka, precyzja, jednostka
   - `setMode(unit)` — ustawia jednostkę wyświetlaną
   - `updateValue(text)` — aktualizacja wartości (nie miga dzięki double-bufferowi)

3. **ProgressBar** — pasek postępu 0-100%
   - `setProgress(percent)` — ustawienie wartości

4. **loop()** — idealne miejsce na ciągłą aktualizację danych (polling z throttlingiem)
