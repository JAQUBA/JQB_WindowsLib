# Chart — Wykres czasu rzeczywistego

> `#include <UI/Chart/Chart.h>`

## Opis

Wykres liniowy czasu rzeczywistego z:
- Automatycznym skalowaniem osi Y
- Konfigurowalnym oknem czasowym osi X
- Siatką, osiami i etykietami
- Ograniczaniem częstotliwości odświeżania (rate limiting)
- Automatycznym usuwaniem starych danych

## Konstruktor

```cpp
Chart(int x, int y, int width, int height, const char* title = "Wykres pomiarów");
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `x`, `y` | `int` | Pozycja |
| `width`, `height` | `int` | Rozmiar |
| `title` | `const char*` | Tytuł wykresu (wyświetlany u góry) |

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę wykresu |
| `addDataPoint(double value, const wstring& unit)` | `void` | Dodaje punkt pomiarowy |
| `clear()` | `void` | Czyści dane |
| `render(HDC hdc)` | `void` | Rysuje wykres (wewnętrzne) |
| `setTimeWindow(int seconds)` | `void` | Okno czasowe osi X (dom. 30 s) |
| `setColors(COLORREF grid, axis, data)` | `void` | Kolory wykresu |
| `setAutoScale(bool)` | `void` | Auto-skalowanie osi Y (dom. true) |
| `setYRange(double min, double max)` | `void` | Ręczny zakres Y |
| `setRefreshRate(int ms)` | `void` | Min. interwał między odświeżeniami (dom. 100 ms) |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 5000) |

## Struktura `DataPoint`

```cpp
struct DataPoint {
    double value;                                 // Wartość
    std::chrono::steady_clock::time_point timestamp; // Czas dodania
    std::wstring unit;                            // Jednostka (np. L"V")
};
```

## Przykłady

### Wykres pomiarów napięcia

```cpp
Chart* chart = new Chart(20, 100, 560, 250, "Napięcie [V]");
chart->setTimeWindow(60);              // Ostatnie 60 sekund
chart->setColors(
    RGB(80, 80, 80),      // siatka
    RGB(200, 200, 200),   // osie
    RGB(0, 255, 0)        // dane
);
window->add(chart);

// Dodawanie danych (np. w callbacku odbierania):
chart->addDataPoint(12.45, L"V");
chart->addDataPoint(12.52, L"V");
```

### Ręczne skalowanie osi Y

```cpp
chart->setAutoScale(false);
chart->setYRange(0.0, 25.0);  // Zakres 0–25
```

### Szybkie odświeżanie

```cpp
chart->setRefreshRate(50);  // Odświeżaj max co 50 ms (20 FPS)
```

## Rysowanie

Wykres używa własnej klasy okna (`ChartClass`) z procedurą `WM_PAINT`:
- Czarne tło
- Siatka (linie kropkowane)
- Osie z etykietami (wartości Y, czas X w sekundach)
- Linia łącząca punkty danych
- Kropki w punktach danych
- Tytuł u góry

## Uwagi

- ID zaczynają się od **5000**
- Dane starsze niż `timeWindow` są automatycznie usuwane
- `addDataPoint()` respektuje `refreshRate` — danych może być więcej niż odświeżeń
- Auto-scale dodaje 10% marginesu powyżej/poniżej wartości ekstremalnych
- Dla płaskiej linii (min ≈ max) automatycznie dodawany jest margines
