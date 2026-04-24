# Example 08 - Engineering Canvas App (gerber2gcode-inspired)

Production-style pattern for tools that combine:

- many form inputs
- side layer panel with grouped toggles
- expensive compute pipeline
- custom canvas rendering
- background worker thread

This pattern is ideal for CAM, CAD, and inspection utilities.

## Suggested File Layout

```text
src/
├── main.cpp
├── AppState.h / AppState.cpp
├── AppUI.h / AppUI.cpp
└── Canvas/MyCanvas.h / Canvas/MyCanvas.cpp
```

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <Util/ConfigManager.h>

#include "AppState.h"
#include "AppUI.h"

void setup() {
    g_window = new SimpleWindow(1200, 820, "Engineering Tool", 101);
    g_window->init();
    g_window->setBackgroundColor(RGB(34, 37, 46));
    g_window->setTextColor(RGB(226, 230, 239));

    g_settings = new ConfigManager("app.ini");

    createUI(g_window);
    loadSettings();

    logMsg("Engineering Tool ready");

    g_window->onClose([]() {
        saveSettings();
    });
}

void loop() {}
```

## `src/AppState.h`

```cpp
#pragma once

#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/TextArea/TextArea.h>
#include <UI/ProgressBar/ProgressBar.h>
#include <UI/InputField/InputField.h>
#include <UI/TreePanel/TreePanel.h>
#include <Util/ConfigManager.h>

#include <string>

class MyCanvas;

extern SimpleWindow* g_window;
extern ConfigManager* g_settings;
extern TextArea* g_log;
extern ProgressBar* g_progress;
extern InputField* g_fldTolerance;
extern MyCanvas* g_canvas;

extern HWND g_hLayerPanel;
extern TreePanel* g_tree;

extern bool g_running;
extern bool g_layerOutline;
extern bool g_layerData;

void loadSettings();
void saveSettings();
void logMsg(const std::string& msg);

void scheduleAutoRefresh();
void doRefreshPreview();
void doGenerateHeavy();
void rebuildLayerPanel();
```

## `src/AppState.cpp`

```cpp
#include "AppState.h"
#include "Canvas/MyCanvas.h"

#include <Util/TimerUtils.h>
#include <windows.h>

SimpleWindow* g_window = nullptr;
ConfigManager* g_settings = nullptr;
TextArea* g_log = nullptr;
ProgressBar* g_progress = nullptr;
InputField* g_fldTolerance = nullptr;
MyCanvas* g_canvas = nullptr;

HWND g_hLayerPanel = NULL;
TreePanel* g_tree = nullptr;

bool g_running = false;
bool g_layerOutline = true;
bool g_layerData = true;

static const UINT_PTR TIMER_PREVIEW = 9601;
static const UINT PREVIEW_DELAY_MS = 400;

void logMsg(const std::string& msg) {
    if (g_log) g_log->append(msg + "\r\n");
}

void loadSettings() {
    if (!g_settings) return;
    if (g_fldTolerance)
        g_fldTolerance->setText(g_settings->getValue("tolerance", "0.05").c_str());
}

void saveSettings() {
    if (!g_settings) return;
    if (g_fldTolerance)
        g_settings->setValue("tolerance", g_fldTolerance->getText());
}

void scheduleAutoRefresh() {
    TimerUtils::restartDebounceTimer(g_window->getHandle(), TIMER_PREVIEW, PREVIEW_DELAY_MS);
}

void doRefreshPreview() {
    if (!g_canvas) return;
    // Recompute lightweight preview data here.
    g_canvas->setShowOutline(g_layerOutline);
    g_canvas->setShowData(g_layerData);
    g_canvas->redraw();
}

static DWORD WINAPI WorkerThreadProc(LPVOID) {
    logMsg("Starting heavy generation...");

    // Simulate heavy pipeline work.
    Sleep(1200);

    logMsg("Generation complete");
    g_running = false;
    if (g_progress) g_progress->setMarquee(false);
    return 0;
}

void doGenerateHeavy() {
    if (g_running) {
        logMsg("Generation already running");
        return;
    }

    g_running = true;
    if (g_progress) g_progress->setMarquee(true);

    HANDLE h = CreateThread(NULL, 0, WorkerThreadProc, NULL, 0, NULL);
    if (h) CloseHandle(h);
}

void rebuildLayerPanel() {
    if (!g_tree) return;

    static bool secLayers = true;

    g_tree->clear();
    g_tree->addSection(L"Layers", &secLayers);
    if (secLayers) {
        g_tree->addItem(L"Outline", &g_layerOutline, 1, []() { doRefreshPreview(); });
        g_tree->addItem(L"Data", &g_layerData, 1, []() { doRefreshPreview(); });
    }
}
```

## `src/AppUI.cpp`

```cpp
#include "AppUI.h"
#include "AppState.h"
#include "Canvas/MyCanvas.h"

#include <Core.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/InputField/InputField.h>
#include <windows.h>

void createUI(SimpleWindow* win) {
    win->add(new Label(12, 10, 220, 20, L"Tolerance:"));

    g_fldTolerance = new InputField(90, 8, 80, 24, "0.05",
        [](InputField*, const char*) {
            scheduleAutoRefresh();
        });
    win->add(g_fldTolerance);

    win->add(new Button(190, 8, 130, 24, "Generate", [](Button*) {
        doGenerateHeavy();
    }));

    g_progress = new ProgressBar(330, 8, 220, 24);
    win->add(g_progress);

    g_canvas = new MyCanvas();
    g_canvas->create(win->getHandle(), 12, 40, 860, 730);

    g_hLayerPanel = CreateWindowExW(0, L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        884, 40, 290, 560,
        win->getHandle(), (HMENU)(intptr_t)9500, _core.hInstance, NULL);

    g_tree = new TreePanel(g_hLayerPanel);
    rebuildLayerPanel();

    g_log = new TextArea(884, 610, 290, 160, nullptr);
    win->add(g_log);

    SetWindowSubclass(win->getHandle(),
        [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) -> LRESULT {
            if (msg == WM_TIMER && wParam == 9601) {
                KillTimer(hwnd, 9601);
                doRefreshPreview();
                return 0;
            }

            if (msg == WM_COMMAND && LOWORD(wParam) == 9500 && HIWORD(wParam) == LBN_SELCHANGE) {
                int idx = (int)SendMessageW(g_hLayerPanel, LB_GETCURSEL, 0, 0);
                if (g_tree && g_tree->handleClick(idx)) rebuildLayerPanel();
                return 0;
            }
            return DefSubclassProc(hwnd, msg, wParam, lParam);
        },
        1, 0);
}
```

## Why This Pattern Works

1. `main.cpp` stays tiny and stable.
2. `AppState` centralizes pointers and behavior.
3. `AppUI` only composes controls and routing.
4. `CreateThread` isolates expensive operations.
5. Debounced refresh keeps the UI responsive while typing.
6. `TreePanel` scales to many toggles better than separate checkboxes.
