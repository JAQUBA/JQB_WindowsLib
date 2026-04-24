# Theme

`UI/Theme/Theme.h` — runtime color palette + helpers to apply it to `SimpleWindow` and individual buttons. Coexists with the older macro-based palette headers in the same folder; both can be used independently.

## Why

A single `Theme` value carries the colors and base font for an entire app. Pass it to helpers — they push colors into `SimpleWindow` background, label/textarea/inputfield default colors, and per-button owner-draw state.

## Struct

```cpp
struct Theme {
    COLORREF bg, surface, surface2;
    COLORREF text, textDim;
    COLORREF accent, accentHover;
    COLORREF ok, warn, err;
    const wchar_t* fontName;
    int            fontSize;
};
```

- `bg` — main window background
- `surface` / `surface2` — card / inset backgrounds
- `text` / `textDim` — primary / secondary text
- `accent` / `accentHover` — call-to-action colors
- `ok` / `warn` / `err` — status colors

## Built-in Palettes

Static factory methods — pick one and pass to `applyTheme`:

| Factory | Family |
|---|---|
| `Theme::catppuccinMocha()` | dark — Catppuccin Mocha |
| `Theme::catppuccinFrappe()` | dark — Catppuccin Frappé |
| `Theme::catppuccinLatte()` | light — Catppuccin Latte |
| `Theme::nord()` | dark — Nord |
| `Theme::dracula()` | dark — Dracula |
| `Theme::tokyoNight()` | dark — Tokyo Night |
| `Theme::oneDark()` | dark — One Dark |
| `Theme::gruvboxDark()` | dark — Gruvbox |
| `Theme::light()` | neutral light |

## Helpers

```cpp
void applyTheme         (SimpleWindow* window, const Theme& t);
void styleAccentButton  (Button* button,       const Theme& t);
void styleSecondaryButton(Button* button,      const Theme& t);
```

`applyTheme()`:

- sets the window background to `bg`
- sets the default static text color to `text`
- iterates `getButtons()` and applies `styleSecondaryButton()` to any button that does not have custom colors set
- iterates `getComponents()` and recolors all `ProgressBar` instances to the accent / surface2 palette

`styleAccentButton(btn, theme)` — fill = `accent`, hover = `accentHover`, text = `bg`. Use for the primary action on each tab/section.

`styleSecondaryButton(btn, theme)` — fill = `surface2`, hover = `surface`, text = `text`. Default for everything else.

## Example

```cpp
#include <UI/Theme/Theme.h>

Theme theme = Theme::catppuccinMocha();

SimpleWindow* w = new SimpleWindow(960, 620, "App", 0);
w->init();
applyTheme(w, theme);

Button* connect = new Button(40, 40, 130, 36, "Polacz", [](Button*){});
w->add(connect);
styleAccentButton(connect, theme);   // primary action

Button* cancel  = new Button(180, 40, 130, 36, "Anuluj", [](Button*){});
w->add(cancel);
styleSecondaryButton(cancel, theme); // optional — applyTheme already does it
```

## Backwards Compatibility

The legacy macro-based headers (`Theme/CatppuccinMocha.h`, etc.) are unchanged. The new struct API is additive — existing code that uses macros continues to work.

## Required SimpleWindow accessors

`applyTheme()` reads these public const accessors added to `SimpleWindow`:

- `getComponents()` — `const std::vector<UIComponent*>&`
- `getButtons()` — `const std::vector<Button*>&`
- `getLabels()` / `getTextAreas()`
- `getBackgroundColor()` / `getDefaultTextColor()`

These are read-only. They are safe to call from your own code.
