# ProgressBar — Pasek postępu

> `#include <UI/ProgressBar/ProgressBar.h>`

## Opis

Natywny pasek postępu Windows z obsługą standardowego trybu (0–100%) i trybu Marquee (animacja nieokreślonego postępu).

## Konstruktor

```cpp
ProgressBar(int x, int y, int width, int height,
            int minimum = 0, int maximum = 100);
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar |
| `minimum` | `int` | Wartość minimalna (domyślnie 0) |
| `maximum` | `int` | Wartość maksymalna (domyślnie 100) |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę `PROGRESS_CLASS` |
| `setProgress(int value)` | `void` | Ustawia wartość (obcinana do zakresu) |
| `getProgress()` | `int` | Pobiera aktualną wartość |
| `setRange(int min, int max)` | `void` | Zmienia zakres |
| `getRange(int& min, int& max)` | `void` | Pobiera zakres |
| `setColor(COLORREF color)` | `void` | Kolor paska |
| `setBackColor(COLORREF color)` | `void` | Kolor tła |
| `setMarquee(bool enable, int interval)` | `void` | Tryb Marquee (animacja) |
| `step(int increment)` | `void` | Inkrementuje wartość |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 6000) |

## Przykłady

### Pasek procentowy

```cpp
ProgressBar* pb = new ProgressBar(20, 50, 300, 25, 0, 100);
window->add(pb);

pb->setProgress(50);   // 50%
pb->step(10);          // 60%
pb->setColor(RGB(0, 200, 0));  // zielony
```

### Tryb Marquee (nieokreślony postęp)

```cpp
ProgressBar* pb = new ProgressBar(20, 50, 300, 25);
window->add(pb);
pb->setMarquee(true, 30);   // Animacja co 30 ms

// Po zakończeniu operacji:
pb->setMarquee(false);
pb->setProgress(100);
```

### Zmiana zakresu

```cpp
pb->setRange(0, 1000);
pb->setProgress(500);   // 50%
```

## Uwagi

- ID zaczynają się od **6000**
- `setColor()` / `setBackColor()` wykorzystują `PBM_SETBARCOLOR` / `PBM_SETBKCOLOR`
- Tryb Marquee dodaje styl `PBS_MARQUEE` do kontrolki
- `step(n)` używa `PBM_DELTAPOS` — dodaje `n` do bieżącej wartości
