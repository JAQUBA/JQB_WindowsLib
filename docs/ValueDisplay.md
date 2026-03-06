# ValueDisplay — Value Display (LCD style)

> `#include <UI/ValueDisplay/ValueDisplay.h>`

## Description

Custom control rendering a numeric value in LCD display style. Ideal for displaying measurements with units, prefixes, and statuses. Supports:
- Large, readable digits (GDI)
- Prefix + unit (e.g. "mV", "kΩ")
- Statuses: AUTO, HOLD, DELTA + custom
- Double buffering (no flicker)
- Configurable appearance (colors, fonts, precision)
- Custom value formatter

## Constructor

```cpp
ValueDisplay(int x, int y, int width, int height);
```

## `DisplayConfig` Structure

```cpp
struct DisplayConfig {
    COLORREF backgroundColor = RGB(20, 20, 20);   // Background (black)
    COLORREF textColor = RGB(0, 220, 0);           // Text (green)
    COLORREF holdTextColor = RGB(220, 0, 0);       // HOLD text (red)
    COLORREF deltaTextColor = RGB(0, 0, 220);      // DELTA text (blue)
    int precision = 2;                              // Decimal places
    std::wstring fontName = L"Arial";               // Font
    double valueFontRatio = 0.6;                    // Value font size / height
    double unitFontRatio = 0.25;                    // Unit font size ratio
    double statusFontRatio = 0.16;                  // Status font size ratio
};
```

## Methods

### Value Updates

| Method | Description |
|--------|-------------|
| `updateValue(double value, wstring prefix, wstring unit)` | Set value with prefix and unit |
| `setMode(const wstring& mode)` | Set mode label (e.g. L"DC V") |
| `setMode(uint8_t mode)` | Set mode numerically (compatibility) |
| `setRange(uint8_t range)` | Set range numerically |
| `setAuto(bool isAuto)` | Toggle AUTO status |
| `setHold(bool isHold)` | Toggle HOLD status |
| `setDelta(bool isDelta)` | Toggle DELTA status |
| `addCustomStatus(wstring name, bool active)` | Add custom status |
| `updateDisplay(value, mode, range, auto, hold, delta)` | Update everything at once (compatibility) |
| `updateFullDisplay(value, prefix, unit, mode, statuses)` | Full update (universal) |

### Configuration

| Method | Description |
|--------|-------------|
| `setConfig(const DisplayConfig& config)` | Set display configuration |
| `setValueFormatter(ValueFormatter formatter)` | Custom value formatting |

### Getters

| Method | Returns |
|--------|---------|
| `getValue()` | `double` — current value |
| `getMode()` | `uint8_t` — numeric mode |
| `getModeString()` | `const wstring&` — mode label |
| `getUnit()` | `const wstring&` — unit |
| `getPrefix()` | `const wstring&` — prefix |
| `getHandle()` | `HWND` |
| `getId()` | `int` (auto from 5000) |

## Custom Formatter

```cpp
using ValueFormatter = std::function<std::wstring(double value, int precision)>;
```

## Examples

### Voltage Display

```cpp
ValueDisplay* vd = new ValueDisplay(20, 20, 300, 120);
window->add(vd);

vd->setMode(L"DC V");
vd->updateValue(12.345, L"", L"V");
```

### Display with Prefix

```cpp
vd->updateValue(4.72, L"m", L"A");    // 4.72 mA
vd->updateValue(2.2, L"k", L"\u03A9"); // 2.2 kΩ
```

### Appearance Configuration

```cpp
ValueDisplay::DisplayConfig config;
config.backgroundColor = RGB(0, 0, 30);
config.textColor = RGB(0, 255, 255);    // cyan
config.precision = 3;
config.fontName = L"Consolas";
vd->setConfig(config);
```

### Custom Formatter

```cpp
vd->setValueFormatter([](double value, int precision) -> std::wstring {
    if (value > 9999) return L"OVER";
    if (value < -9999) return L"UNDER";
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
});
```

### Full Update

```cpp
std::map<std::wstring, bool> statuses;
statuses[L"AUTO"] = true;
statuses[L"RMS"] = true;

vd->updateFullDisplay(
    230.5,             // value
    L"",               // prefix
    L"V",              // unit
    L"AC V",           // mode
    statuses           // statuses
);
```

## Display Layout

```
┌────────────────────────────────────┐
│ AUTO HOLD DELTA          DC V      │  ← statuses (top left), mode (top right)
│                                    │
│                                    │
│            12.345 V                │  ← value + unit (bottom, centered)
└────────────────────────────────────┘
```

## Notes

- Control registers its own window class `ValueDisplayClass`
- Double buffering (`CreateCompatibleDC` / `BitBlt`) eliminates flicker
- `WM_ERASEBKGND` returns 1 (background drawn in `WM_PAINT`)
- Text colors change automatically: HOLD=red, DELTA=blue, normal=green
- `INFINITY` is formatted as `"OL"` (Over Limit)
