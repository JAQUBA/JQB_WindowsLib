# Example 09 - Document Editor + Preview Canvas (WektoroweLitery2-inspired)

Pattern for desktop tools that combine:

- text/document editor
- live preview canvas
- debounced re-render
- export action

Useful for generators, configuration designers, and conversion utilities.

Runnable project version:

- [../../examples/04_document_editor_preview](../../examples/04_document_editor_preview/README.md)

## Scenario

- Left side: editable text document.
- Right side: custom canvas preview.
- Top bar: file actions + parameters.
- Preview refreshes automatically after typing stops.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>

#include "AppState.h"
#include "AppUI.h"
#include "Canvas/MyDocCanvas.h"

MyDocCanvas* g_canvas = nullptr;

void setup() {
    loadSettings();

    window = new SimpleWindow(1200, 700, "Document Preview Tool", 101);
    window->init();

    createUI(window);

    g_canvas = new MyDocCanvas();
    g_canvas->create(window->getHandle(), 0, 0, 100, 100);

    doRelayout();
    doRenderPreview();

    window->onClose([]() {
        saveSettings();
        if (g_canvas) {
            delete g_canvas;
            g_canvas = nullptr;
        }
    });
}

void loop() {}
```

## `src/AppState.h`

```cpp
#pragma once

#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <string>

class MyDocCanvas;

extern SimpleWindow* window;
extern Label* lblInfo;
extern HWND hEditor;
extern MyDocCanvas* g_canvas;

extern std::string exportOutput;
extern int editorWidth;

void loadSettings();
void saveSettings();

std::string getEditorText();
void setEditorText(const std::string& text);

void doRenderPreview();
void doExport();
void doRelayout();
```

## `src/AppUI.cpp`

```cpp
#include "AppUI.h"
#include "AppState.h"

#include <Core.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/InputField/InputField.h>
#include <commctrl.h>
#include <richedit.h>

static HWND hSplitter = NULL;
static bool splitterDragging = false;
static int dragStartX = 0;
static int dragStartWidth = 0;

static const UINT_PTR TIMER_RENDER = 42;
static const UINT RENDER_DELAY_MS = 300;

static LRESULT CALLBACK ParentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                                   UINT_PTR, DWORD_PTR) {
    if (msg == WM_COMMAND && HIWORD(wParam) == EN_CHANGE && (HWND)lParam == hEditor) {
        KillTimer(hwnd, TIMER_RENDER);
        SetTimer(hwnd, TIMER_RENDER, RENDER_DELAY_MS, NULL);
    }

    if (msg == WM_TIMER && wParam == TIMER_RENDER) {
        KillTimer(hwnd, TIMER_RENDER);
        doRenderPreview();
        return 0;
    }

    if (msg == WM_SIZE) {
        doRelayout();
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK SplitterProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_LBUTTONDOWN: {
            splitterDragging = true;
            dragStartWidth = editorWidth;
            POINT pt;
            GetCursorPos(&pt);
            dragStartX = pt.x;
            SetCapture(hwnd);
            return 0;
        }
        case WM_MOUSEMOVE:
            if (splitterDragging) {
                POINT pt;
                GetCursorPos(&pt);
                editorWidth = dragStartWidth + (pt.x - dragStartX);
                if (editorWidth < 180) editorWidth = 180;
                if (editorWidth > 800) editorWidth = 800;
                doRelayout();
            }
            return 0;
        case WM_LBUTTONUP:
            if (splitterDragging) {
                splitterDragging = false;
                ReleaseCapture();
            }
            return 0;
        case WM_SETCURSOR:
            SetCursor(LoadCursorW(NULL, (LPCWSTR)IDC_SIZEWE));
            return TRUE;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void createUI(SimpleWindow* win) {
    win->add(new Button(10, 8, 120, 26, "Render", [](Button*) { doRenderPreview(); }));
    win->add(new Button(140, 8, 120, 26, "Export", [](Button*) { doExport(); }));

    lblInfo = new Label(280, 10, 700, 22, L"Ready");
    win->add(lblInfo);

    LoadLibraryW(L"Msftedit.dll");

    hEditor = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"RICHEDIT50W", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL |
        ES_WANTRETURN | WS_VSCROLL | WS_HSCROLL,
        10, 44, 360, 620,
        win->getHandle(), NULL, _core.hInstance, NULL);

    SendMessageW(hEditor, EM_SETEVENTMASK, 0,
        SendMessageW(hEditor, EM_GETEVENTMASK, 0, 0) | ENM_CHANGE);

    // Splitter registration omitted for brevity in this doc.
    // Register a tiny custom window class and use SplitterProc as WndProc.

    SetWindowSubclass(win->getHandle(), ParentProc, 1, 0);
}
```

## `src/AppState.cpp` (core behavior)

```cpp
#include "AppState.h"
#include "Canvas/MyDocCanvas.h"

#include <Util/ConfigManager.h>
#include <Util/StringUtils.h>
#include <windows.h>
#include <fstream>

SimpleWindow* window = nullptr;
Label* lblInfo = nullptr;
HWND hEditor = NULL;

std::string exportOutput = "";
int editorWidth = 360;

static ConfigManager config("app.ini");

extern MyDocCanvas* g_canvas;

void loadSettings() {
    exportOutput = config.getValue("output_file", "");
    try { editorWidth = std::stoi(config.getValue("editor_width", "360")); }
    catch (...) { editorWidth = 360; }
}

void saveSettings() {
    config.setValue("output_file", exportOutput);
    config.setValue("editor_width", std::to_string(editorWidth));
}

std::string getEditorText() {
    if (!hEditor) return "";
    int len = GetWindowTextLengthW(hEditor);
    std::wstring w(len + 1, L'\0');
    GetWindowTextW(hEditor, &w[0], len + 1);
    w.resize(len);
    return StringUtils::wideToUtf8(w);
}

void setEditorText(const std::string& text) {
    if (!hEditor) return;
    std::wstring w = StringUtils::utf8ToWide(text);
    SetWindowTextW(hEditor, w.c_str());
}

void doRenderPreview() {
    if (!g_canvas) return;

    std::string doc = getEditorText();
    g_canvas->setSource(doc);
    g_canvas->redraw();

    if (lblInfo) lblInfo->setText(L"Preview updated");
}

void doExport() {
    if (lblInfo) lblInfo->setText(L"Exported");
}

void doRelayout() {
    if (!window || !g_canvas || !hEditor) return;

    RECT rc;
    GetClientRect(window->getHandle(), &rc);

    int top = 44;
    int h = (rc.bottom - top) - 10;
    int splitterW = 6;

    MoveWindow(hEditor, 10, top, editorWidth - 10, h, TRUE);

    int canvasX = editorWidth + splitterW + 10;
    int canvasW = rc.right - canvasX - 10;
    if (canvasW < 50) canvasW = 50;

    MoveWindow(g_canvas->getHandle(), canvasX, top, canvasW, h, TRUE);
}
```

## Why This Pattern Works

1. RichEdit gives practical text editing with multiline behavior.
2. Debounced preview avoids expensive redraw per keystroke.
3. Splitter gives user-controlled workspace balance.
4. Canvas remains isolated in its own subclass.
5. Export path is cleanly separated from render logic.
