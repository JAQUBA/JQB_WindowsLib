# UI Design Guide

How to build **good-looking, consistent** desktop applications on top of `JQB_WindowsLib`.
This guide is the prescriptive companion to [Theme.md](Theme.md), [SimpleWindow.md](SimpleWindow.md), and the per-control docs. It distils the patterns proven in [WektoroweLitery2](https://github.com/JAQUBA/WektoroweLitery2) and [gerber2gcode](https://github.com/JAQUBA/gerber2gcode).

> **TL;DR** — pick a `Theme`, call `applyTheme(window, theme)` after `init()`, group controls into **cards**, give every section an **accent header** + **dim field labels**, mark the primary action of each section with `styleAccentButton()`, and route long work to a worker thread so the UI stays responsive.

---

## 1. Layout principles

### 1.1 Hierarchy

Every screen has three visual tiers:

| Tier | Purpose | Implementation |
|---|---|---|
| **Surface** | The window/page background | `theme.bg` (set by `applyTheme`) |
| **Card** | A grouped block of related controls | An empty `Label` filled with `theme.surface` |
| **Field row** | One labelled input | `fieldLabel` + control aligned on a column |

Never place inputs directly on the bare window background — always wrap them in a card. Cards give the eye a place to rest and stop the layout from looking like a 1995 dialog.

### 1.2 The 8/16/24 rule

Use only multiples of **8 px** for paddings, **16 px** between rows inside a card, and **24 px** between cards. This keeps every screen visually aligned without measuring.

```
window edge  → card edge      : 24 px
card edge    → first label    : 16 px (top), 16 px (left)
row to row inside a card      : 32 px (16 above + 16 below)
card to card                  : 24 px
```

### 1.3 Alignment grid

Inside a card, fix two vertical guides:

- `xLbl` — left edge for `fieldLabel`
- `xCtl` — left edge for the input control

Never let labels and controls drift; reading speed depends on that grid.

```cpp
const int xLbl = 40, xCtl = 200, h = 26;
int y = 70;

fieldLabel(page, xLbl, y + 4, 120, L"Baud rate");  // +4 centers label vs 26-px tall edit
new Select(xCtl, y, 220, h, "9600", ...);
y += 32;  // next row
```

---

## 2. Colour and typography

### 2.1 Use the `Theme` palette — never hard-code colours

```cpp
#include <UI/Theme/Theme.h>

m_theme = Theme::catppuccinMocha();   // pick once
applyTheme(m_window, m_theme);        // background + buttons + tab pages + dark titlebar
```

Built-in palettes: `catppuccinMocha/Frappe/Latte`, `nord`, `dracula`, `tokyoNight`, `oneDark`, `gruvboxDark`, `light`.

| Slot | Use it for |
|---|---|
| `bg` | Window background, tab page background |
| `surface` | Card background (one step lighter than `bg`) |
| `surface2` | Hover, secondary buttons, input fill |
| `text` | Primary text on `bg`/`surface` |
| `textDim` | Field labels, footer text, helper hints |
| `accent` | Section headers, primary buttons, focus underline |
| `accentHover` | Hover state of accent buttons |
| `ok` / `warn` / `err` | Status dot, log severity, validation badges |

> **Exception:** physical I/O semantics (e.g. ON = warm yellow, OFF = dim grey) may use hard-coded `RGB(...)` because they describe state, not branding.

### 2.2 Fonts

`Theme` defaults to `Segoe UI` 10 pt. Stay there for body text. Use **bold** for section headers and accent buttons, **regular** for everything else:

```cpp
sectionTitle->setFont(L"Segoe UI", 11, true);   // bold, +1pt
fieldLabel  ->setFont(L"Segoe UI", 10, false);  // dim, normal
```

Use `Consolas` 11 pt for all log/console areas (`TextArea`):

```cpp
log->setFont(L"Consolas", 11, false);
log->setTextColor(theme.textDim);
log->setBackColor(theme.surface);
```

### 2.3 Dark title bar

`applyTheme()` heuristically enables Win32 immersive dark mode (`DwmSetWindowAttribute(20, ...)`) when the palette is dark. No extra call needed.

---

## 3. Cards, sections, and field labels

Three tiny helpers that you should drop into every `AppUI`:

```cpp
// === in AppUI.h ===
class AppUI {
private:
    Label* card        (HWND parent, int x, int y, int w, int h);
    Label* sectionTitle(HWND parent, int x, int y, int w, const wchar_t* text);
    Label* fieldLabel  (HWND parent, int x, int y, int w, const wchar_t* text);
    Theme  m_theme;
};

// === in AppUI.cpp ===
Label* AppUI::card(HWND parent, int x, int y, int w, int h) {
    Label* l = new Label(x, y, w, h, L"");
    m_window->add(l, parent);
    l->setBackColor(m_theme.surface);
    return l;
}

Label* AppUI::sectionTitle(HWND parent, int x, int y, int w, const wchar_t* text) {
    Label* l = new Label(x, y, w, 22, text);
    m_window->add(l, parent);
    l->setFont(L"Segoe UI", 11, true);
    l->setTextColor(m_theme.accent);
    return l;
}

Label* AppUI::fieldLabel(HWND parent, int x, int y, int w, const wchar_t* text) {
    Label* l = new Label(x, y, w, 20, text);
    m_window->add(l, parent);
    l->setFont(L"Segoe UI", 10, false);
    l->setTextColor(m_theme.textDim);
    return l;
}
```

Use them like this:

```cpp
card        (page, 24, 24, 440, 230);
sectionTitle(page, 40, 36, 200, L"Parametry portu");
fieldLabel  (page, 40, 76, 120, L"Baud rate");
new Select  (200,   72, 220, 26, "9600", ...);
```

> **Order matters.** Add the card **first** so it paints behind the controls. Library `Label` controls are drawn in z-order of `add()`.

---

## 4. Buttons

Each section has **at most one primary action** — that's the one styled with the accent. Everything else is secondary.

```cpp
auto* ok = new Button(x, y, 140, 36, "Polacz", [this](Button*){ doConnect(); });
m_window->add(ok, page);
styleAccentButton(ok, m_theme);   // accent fill, dark text, bold

auto* cancel = new Button(x + 160, y, 140, 36, "Rozlacz", [this](Button*){ doDisconnect(); });
m_window->add(cancel, page);
// no styleSecondaryButton call needed — applyTheme() did it automatically
```

Button sizing reference:

| Use | Size |
|---|---|
| Small inline (Scan, Send) | `100 × 26` |
| Standard (Connect, Save)  | `140 × 36` |
| Wide primary (Generate)   | `200 × 40` |
| Toolbar / icon-only        | `28 × 28`  |

Always provide the **textual** action name. Avoid icon-only buttons unless space is critical, and never rely on colour alone to convey state.

---

## 5. Tabs

`TabControl` is the right tool when one app has 4–10 distinct workflows. Below or above that, prefer a single page or a side panel.

```cpp
m_tabs = new TabControl(0, 0, kWinW, kTabsH);
m_window->add(m_tabs);
m_tabs->addTab(" Polaczenie ");   // pad with spaces so text breathes
m_tabs->addTab(" Nastawy ");
// ...
buildConnectionTab(m_tabs->getTabPage(0));
buildSettingsTab  (m_tabs->getTabPage(1));
```

`applyTheme()` automatically calls `setPageBackground(theme.bg)` on every `TabControl` it finds, so the page area matches the window. (The OS-drawn tab strip itself stays system-coloured — that's a Windows limitation.)

> Tab labels are UTF-8 `char*`. Use ASCII transliteration (`"Polaczenie"`, not `"Połączenie"`) in source literals to keep the build portable across compilers.

---

## 6. Status, footer, and feedback

Every long-running app needs a **footer strip** that answers two questions: *am I connected?* and *what is the app doing right now?*

```cpp
// Status dot — a tiny round Label
m_statusDot = new Label(20, kWinH - 26, 14, 14, L"");
m_window->add(m_statusDot);
m_statusDot->setBackColor(m_theme.err);   // start red

// Status text
m_statusText = new Label(40, kWinH - 26, 400, 14, L"Rozlaczono");
m_window->add(m_statusText);
m_statusText->setTextColor(m_theme.textDim);
```

When state changes, recolor the dot (`theme.ok` / `theme.warn` / `theme.err`) and rewrite the text. The eye finds a colour-coded dot 5× faster than scanning a status line.

For modal progress use `ProgressBar` with `setMarquee(true)`; place it in the footer or above the action button.

---

## 7. Threading and responsiveness

The UI runs on one thread. Anything that takes longer than ~50 ms (Modbus scan, file parse, network call) **must** run on a worker thread, otherwise the title bar shows „Not responding".

```cpp
// Spawn worker — MinGW-friendly (no std::thread)
struct ScanCtx { AppUI* ui; int from; int to; };
auto* ctx = new ScanCtx{this, 1, 247};

CreateThread(NULL, 0, [](LPVOID p) -> DWORD {
    auto* c = (ScanCtx*)p;
    for (int id = c->from; id <= c->to; ++id) {
        // ... blocking I/O ...
        // Push UI updates back to the main thread via PostMessage
        PostMessageW(c->ui->getHandle(), WM_APP + 1, id, found ? 1 : 0);
    }
    delete c;
    return 0;
}, ctx, 0, NULL);
```

Drain those `WM_APP+N` messages from `AppUI::onLoop()` (called via `loop()`) and update the UI synchronously there.

---

## 8. Polling

For periodic background work (read sensors every 200 ms, refresh I/O bits every 1 s), use `PollingManager`. **Don't** use threads for this — `PollingManager` runs on the UI thread, so callbacks may touch widgets directly without locking.

```cpp
m_polling.addGroup("monitor", 1000, [this]() {
    if (!m_state.isConnected()) return;
    for (auto& a : m_analogs) {
        double v = 0;
        if (m_state.readRegister(*a.reg, v))
            a.edit->setText(toStr(v, a.reg->decimals).c_str());
    }
});
```

Then in `loop()`:

```cpp
void loop() { polling.tick(); ui->onLoop(); }
```

A checkbox that toggles polling:

```cpp
new CheckBox(x, y, 100, 22, "AUTO", true,
    [this](CheckBox*, bool on){ m_polling.setEnabled("monitor", on); });
```

---

## 9. Logging

Use `TextLogger` attached to a `TextArea` placed on a dedicated „LOG" tab. The library handles filtering (`info / error / tx / rx`), timestamps, and snapshot export.

```cpp
m_log = new TextArea(0, 0, kPageW, kPageH);
m_window->add(m_log, logPage);
m_log->setFont(L"Consolas", 11, false);
m_log->setTextColor(m_theme.textDim);
m_log->setBackColor(m_theme.surface);

m_logger.attach(m_log);
m_state.logger = &m_logger;   // domain code logs through the same sink
```

Provide a small toolbar above the log: filter checkboxes (`INFO / ERR / TX / RX`), `Wyczysc`, `Eksport`.

---

## 10. Persistence

Every user setting (port, baud, last directory, window size, active tool) belongs in `ConfigManager`. Load in `setup()`, save in the destructor or on `WM_CLOSE`. Never lose user preferences between runs.

```cpp
m_state.serial.baud   = (DWORD)atoi(cfg.getValue("baud",    "9600").c_str());
m_state.modbusId      = (uint8_t)atoi(cfg.getValue("modbus_id", "1").c_str());
m_state.activeProfile = profiles::findProfile(cfg.getValue("profile", "MS10N"));
```

---

## 11. Anti-patterns

Don't do these — they make apps look amateurish:

- ❌ **Hard-coded `RGB(255, 255, 255)`** instead of `theme.text` — breaks theme switching.
- ❌ **Random `+5 / -3` pixel offsets** — use the 8/16/24 rule.
- ❌ **Uppercase "EVERY BUTTON SHOUTING"** — sentence case looks calmer.
- ❌ **Polish diacritics in source string literals** — keep source ASCII (`"Polaczenie"`); render diacritics through resource files if you really must.
- ❌ **Modal `MessageBoxW` for every error** — log to `TextLogger` and surface only fatal errors with a dialog.
- ❌ **Spawning `std::thread`** — use `CreateThread`; `std::thread` doesn't link cleanly under default MinGW.
- ❌ **Manually `delete`-ing a control added with `window->add(...)`** — `SimpleWindow` owns it.
- ❌ **A second `SimpleWindow`** — it's a singleton. Use `OverlayWindow` for floating panels.
- ❌ **Doing Modbus / file I/O directly in `loop()`** — block the UI. Use a worker.
- ❌ **Inputs floating on bare `theme.bg`** — wrap them in a `card()`.

---

## 12. Reference applications

These two apps follow this guide closely. Read their `AppUI.cpp` for live examples.

| App | Pattern highlights |
|---|---|
| [WektoroweLitery2](https://github.com/JAQUBA/WektoroweLitery2) | Tokyo Night theme, toolbar of styled buttons, single-canvas workflow, `LogWindow` detached from main |
| [gerber2gcode](https://github.com/JAQUBA/gerber2gcode) | Multi-row toolbar with section headers (Project/Machining/Position), themed inputs, `TreePanel` side panel, themed listbox via `WM_CTLCOLORLISTBOX`, threaded pipeline, `ProgressBar` in marquee mode |
| [Konfigurator](https://github.com/JAQUBA/Konfigurator) | Catppuccin Mocha, 7-tab layout, cards everywhere, status dot footer, `PollingManager` for live I/O, worker thread for the slave-ID scanner |

---

## 13. Cheat sheet

```cpp
// 1. Pick a theme and apply it after init()
m_theme = Theme::catppuccinMocha();
m_window = new SimpleWindow(960, 620, "MyApp", 0);
m_window->init();
applyTheme(m_window, m_theme);

// 2. Build tabs (page bg auto-themed)
m_tabs = new TabControl(0, 0, 960, 590);
m_window->add(m_tabs);
m_tabs->addTab(" Connection ");
HWND page = m_tabs->getTabPage(0);

// 3. Card → section title → field labels → controls
card        (page,  24,  24, 440, 230);
sectionTitle(page,  40,  36, 200, L"Connection");
fieldLabel  (page,  40,  76, 120, L"Port");
auto* portSel = new Select(200, 72, 220, 26, "COM1", ...);
m_window->add(portSel, page);

// 4. Primary action = accent
auto* btn = new Button(180, 540, 200, 40, "Connect",
                       [this](Button*){ doConnect(); });
m_window->add(btn);
styleAccentButton(btn, m_theme);

// 5. Status footer
auto* dot = new Label(20, 590, 14, 14, L"");
m_window->add(dot);
dot->setBackColor(m_theme.err);

// 6. Long work → CreateThread → PostMessage → drain in onLoop()
// 7. Periodic work → PollingManager group + tick() in loop()
// 8. User settings → ConfigManager (load in setup(), save on close)
```

That's it. Stick to these eight steps and your app will look like it shipped in 2026, not 1996.
