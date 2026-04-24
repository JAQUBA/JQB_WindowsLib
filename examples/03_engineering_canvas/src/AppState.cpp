#include "AppState.h"

#include "Canvas/EngineeringCanvas.h"

#include <UI/TreePanel/TreePanel.h>
#include <Util/NumberUtils.h>
#include <Util/TimerUtils.h>

SimpleWindow* g_window = nullptr;
ConfigManager* g_settings = nullptr;
TextArea* g_log = nullptr;
ProgressBar* g_progress = nullptr;
InputField* g_fldTolerance = nullptr;
EngineeringCanvas* g_canvas = nullptr;

HWND g_hLayerPanel = NULL;
TreePanel* g_tree = nullptr;

bool g_isRunning = false;
bool g_showOutline = true;
bool g_showPaths = true;
bool g_layersExpanded = true;

void logMsg(const std::string& msg) {
    if (!g_log) return;
    g_log->append(msg + "\r\n");
}

void loadSettings() {
    if (!g_settings) return;

    if (g_fldTolerance) {
        g_fldTolerance->setText(g_settings->getValue("tolerance", "0.05").c_str());
    }

    g_showOutline = g_settings->getValue("show_outline", "1") == "1";
    g_showPaths = g_settings->getValue("show_paths", "1") == "1";
    g_layersExpanded = g_settings->getValue("layers_expanded", "1") == "1";

    rebuildLayerPanel();
}

void saveSettings() {
    if (!g_settings) return;

    if (g_fldTolerance) {
        g_settings->setValue("tolerance", g_fldTolerance->getText());
    }

    g_settings->setValue("show_outline", g_showOutline ? "1" : "0");
    g_settings->setValue("show_paths", g_showPaths ? "1" : "0");
    g_settings->setValue("layers_expanded", g_layersExpanded ? "1" : "0");
}

void scheduleAutoRefresh() {
    if (!g_window) return;
    TimerUtils::restartDebounceTimer(g_window->getHandle(), TIMER_PREVIEW, PREVIEW_DELAY_MS);
}

void doRefreshPreview() {
    if (!g_canvas) return;

    double tol = 0.05;
    if (g_fldTolerance) {
        NumberUtils::tryParseDouble(g_fldTolerance->getText(), tol);
    }

    int density = 24 - (int)(tol * 220.0);
    if (density < 4) density = 4;
    if (density > 60) density = 60;

    g_canvas->setShowOutline(g_showOutline);
    g_canvas->setShowPaths(g_showPaths);
    g_canvas->setPathDensity(density);
    g_canvas->redraw();
}

static DWORD WINAPI GenerateThreadProc(LPVOID) {
    logMsg("Generation started...");

    for (int i = 0; i < 3; ++i) {
        Sleep(450);
        logMsg("Working...");
    }

    logMsg("Generation finished");

    g_isRunning = false;
    if (g_progress) g_progress->setMarquee(false);

    return 0;
}

void doGenerateHeavy() {
    if (g_isRunning) {
        logMsg("Generation is already running");
        return;
    }

    g_isRunning = true;
    if (g_progress) g_progress->setMarquee(true);

    HANDLE hThread = CreateThread(NULL, 0, GenerateThreadProc, NULL, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    }
}

void rebuildLayerPanel() {
    if (!g_tree) return;

    g_tree->clear();

    g_tree->addSection(L"Layers", &g_layersExpanded);
    if (g_layersExpanded) {
        g_tree->addItem(L"Board Outline", &g_showOutline, 1, []() {
            doRefreshPreview();
        });

        g_tree->addItem(L"Toolpaths", &g_showPaths, 1, []() {
            doRefreshPreview();
        });

        g_tree->addActionItem(L"Show All", g_showOutline && g_showPaths, []() {
            g_showOutline = true;
            g_showPaths = true;
            doRefreshPreview();
        }, 1);
    }
}
