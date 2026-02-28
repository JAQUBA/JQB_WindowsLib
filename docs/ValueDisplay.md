# ValueDisplay — Wyświetlacz wartości (styl LCD)

> `#include <UI/ValueDisplay/ValueDisplay.h>`

## Opis

Niestandardowa kontrolka rysująca wartość numeryczną w stylu wyświetlacza LCD. Idealna do wyświetlania pomiarów z jednostkami, prefiksami i statusami. Obsługuje:
- Duże, czytelne cyfry (GDI)
- Prefiks + jednostka (np. "mV", "kΩ")
- Statusy: AUTO, HOLD, DELTA + niestandardowe
- Podwójne buforowanie (bez migotania)
- Konfigurowalny wygląd (kolory, czcionki, precyzja)
- Niestandardowy formatter wartości

## Konstruktor

```cpp
ValueDisplay(int x, int y, int width, int height);
```

## Struktura `DisplayConfig`

```cpp
struct DisplayConfig {
    COLORREF backgroundColor = RGB(20, 20, 20);   // Tło (czarne)
    COLORREF textColor = RGB(0, 220, 0);           // Tekst (zielony)
    COLORREF holdTextColor = RGB(220, 0, 0);       // Tekst HOLD (czerwony)
    COLORREF deltaTextColor = RGB(0, 0, 220);      // Tekst DELTA (niebieski)
    int precision = 2;                              // Miejsca po przecinku
    std::wstring fontName = L"Arial";               // Czcionka
    double valueFontRatio = 0.6;                    // Rozmiar czcionki wartości / height
    double unitFontRatio = 0.25;                    // Rozmiar czcionki jednostek
    double statusFontRatio = 0.16;                  // Rozmiar czcionki statusów
};
```

## Metody

### Aktualizacja wartości

| Metoda | Opis |
|--------|------|
| `updateValue(double value, wstring prefix, wstring unit)` | Ustaw wartość z prefiksem i jednostką |
| `setMode(const wstring& mode)` | Ustaw etykietę trybu (np. L"DC V") |
| `setMode(uint8_t mode)` | Ustaw tryb numerycznie (kompatybilność) |
| `setRange(uint8_t range)` | Ustaw zakres numerycznie |
| `setAuto(bool isAuto)` | Włącz/wyłącz status AUTO |
| `setHold(bool isHold)` | Włącz/wyłącz status HOLD |
| `setDelta(bool isDelta)` | Włącz/wyłącz status DELTA |
| `addCustomStatus(wstring name, bool active)` | Dodaj niestandardowy status |
| `updateDisplay(value, mode, range, auto, hold, delta)` | Aktualizuj wszystko naraz (kompatybilność) |
| `updateFullDisplay(value, prefix, unit, mode, statuses)` | Pełna aktualizacja (uniwersalna) |

### Konfiguracja

| Metoda | Opis |
|--------|------|
| `setConfig(const DisplayConfig& config)` | Ustaw konfigurację wyświetlacza |
| `setValueFormatter(ValueFormatter formatter)` | Niestandardowe formatowanie wartości |

### Gettery

| Metoda | Zwraca |
|--------|--------|
| `getValue()` | `double` — aktualna wartość |
| `getMode()` | `uint8_t` — tryb numeryczny |
| `getModeString()` | `const wstring&` — etykieta trybu |
| `getUnit()` | `const wstring&` — jednostka |
| `getPrefix()` | `const wstring&` — prefiks |
| `getHandle()` | `HWND` |
| `getId()` | `int` (auto od 5000) |

## Niestandardowy formatter

```cpp
using ValueFormatter = std::function<std::wstring(double value, int precision)>;
```

## Przykłady

### Wyświetlacz napięcia

```cpp
ValueDisplay* vd = new ValueDisplay(20, 20, 300, 120);
window->add(vd);

vd->setMode(L"DC V");
vd->updateValue(12.345, L"", L"V");
```

### Wyświetlacz z prefiksem

```cpp
vd->updateValue(4.72, L"m", L"A");    // 4.72 mA
vd->updateValue(2.2, L"k", L"Ω");     // 2.2 kΩ
```

### Konfiguracja wyglądu

```cpp
ValueDisplay::DisplayConfig config;
config.backgroundColor = RGB(0, 0, 30);
config.textColor = RGB(0, 255, 255);    // cyan
config.precision = 3;
config.fontName = L"Consolas";
vd->setConfig(config);
```

### Niestandardowy formatter

```cpp
vd->setValueFormatter([](double value, int precision) -> std::wstring {
    if (value > 9999) return L"OVER";
    if (value < -9999) return L"UNDER";
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
});
```

### Pełna aktualizacja

```cpp
std::map<std::wstring, bool> statuses;
statuses[L"AUTO"] = true;
statuses[L"RMS"] = true;

vd->updateFullDisplay(
    230.5,             // wartość
    L"",               // prefiks
    L"V",              // jednostka
    L"AC V",           // tryb
    statuses           // statusy
);
```

## Layout wyświetlacza

```
┌────────────────────────────────────┐
│ AUTO HOLD DELTA          DC V      │  ← statusy (lewy górny), tryb (prawy górny)
│                                    │
│                                    │
│            12.345 V                │  ← wartość + jednostka (dolna część, wyśrodkowana)
└────────────────────────────────────┘
```

## Uwagi

- Kontrolka rejestruje własną klasę okna `ValueDisplayClass`
- Podwójne buforowanie (`CreateCompatibleDC` / `BitBlt`) eliminuje migotanie
- `WM_ERASEBKGND` zwraca 1 (rysowanie tła w `WM_PAINT`)
- Kolory tekstu zmieniają się automatycznie: HOLD=czerwony, DELTA=niebieski, normalny=zielony
- `INFINITY` formatowana jest jako `"OL"` (Over Limit)
