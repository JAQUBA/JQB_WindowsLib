#include "AppUI.h"

#include "AppState.h"
#include "Canvas/EngineeringCanvas.h"

#include <Core.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/InputField/InputField.h>
#include <UI/TreePanel/TreePanel.h>
#include <Util/TimerUtils.h>

#include <commctrl.h>

static const int TOOLBAR_H = 44;
static const int MARGIN = 10;
static const int RIGHT_PANEL_W = 300;
static const int LOG_H = 140;

static LRESULT CALLBACK MainSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_SIZE) {
        doResize(LOWORD(lParam), HIWORD(lParam));
        return 0;
    }

    if (msg == WM_TIMER && wParam == TIMER_PREVIEW) {
        TimerUtils::stopTimer(hwnd, TIMER_PREVIEW);
        doRefreshPreview();
        return 0;
    }

    if (msg == WM_COMMAND && LOWORD(wParam) == LAYER_PANEL_ID && HIWORD(wParam) == LBN_SELCHANGE) {
        int idx = (int)SendMessageW(g_hLayerPanel, LB_GETCURSEL, 0, 0);
        if (g_tree && g_tree->handleClick(idx)) {
            rebuildLayerPanel();
        }
        return 0;
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void createUI(SimpleWindow* window) {
    window->add(new Label(12, 11, 64, 22, L"Tol:"));

    g_fldTolerance = new InputField(50, 8, 90, 26, "0.05",
        [](InputField*, const char*) {
            scheduleAutoRefresh();
        });
    window->add(g_fldTolerance);

    window->add(new Button(150, 8, 130, 26, "Generate", [](Button*) {
        doGenerateHeavy();
    }));

    g_progress = new ProgressBar(290, 8, 240, 26);
    window->add(g_progress);

    g_canvas = new EngineeringCanvas();
    g_canvas->create(window->getHandle(), 0, 0, 100, 100);
    g_canvas->setGridVisible(true);
    g_canvas->setBackgroundColor(RGB(18, 22, 31));
    g_canvas->setGridColor(RGB(50, 60, 80));
    g_canvas->zoomToFit(120.0, 80.0);

    g_hLayerPanel = CreateWindowExW(
        0, L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        0, 0, 100, 100,
        window->getHandle(), (HMENU)(intptr_t)LAYER_PANEL_ID, _core.hInstance, NULL);

    HFONT listFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, 0, 0, 0,
        DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    SendMessageW(g_hLayerPanel, WM_SETFONT, (WPARAM)listFont, TRUE);
    SendMessageW(g_hLayerPanel, LB_SETITEMHEIGHT, 0, 20);

    g_tree = new TreePanel(g_hLayerPanel);
    rebuildLayerPanel();

    g_log = new TextArea(0, 0, 120, 80);
    window->add(g_log);

    SetWindowSubclass(window->getHandle(), MainSubclassProc, 1, 0);

    RECT rc;
    GetClientRect(window->getHandle(), &rc);
    doResize(rc.right, rc.bottom);
}

void doResize(int width, int height) {
    if (!g_canvas || !g_hLayerPanel || !g_log) return;

    int panelX = width - RIGHT_PANEL_W - MARGIN;
    int bodyY = TOOLBAR_H + MARGIN;

    int logY = height - LOG_H - MARGIN;
    int layerH = logY - bodyY - MARGIN;
    if (layerH < 80) layerH = 80;

    int canvasX = MARGIN;
    int canvasW = panelX - canvasX - MARGIN;
    if (canvasW < 200) canvasW = 200;

    int canvasH = height - bodyY - MARGIN;
    if (canvasH < 120) canvasH = 120;

    MoveWindow(g_canvas->getHandle(), canvasX, bodyY, canvasW, canvasH, TRUE);
    MoveWindow(g_hLayerPanel, panelX, bodyY, RIGHT_PANEL_W, layerH, TRUE);
    MoveWindow(g_log->getHandle(), panelX, logY, RIGHT_PANEL_W, LOG_H, TRUE);
}
