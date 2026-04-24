#include <Core.h>

#include "AppState.h"
#include "AppUI.h"

void setup() {
    g_window = new SimpleWindow(1240, 820, "Example 03 - Engineering Canvas", 0);
    g_window->init();
    g_window->setBackgroundColor(RGB(34, 37, 46));
    g_window->setTextColor(RGB(226, 230, 239));

    g_settings = new ConfigManager("engineering_canvas.ini");

    createUI(g_window);
    loadSettings();
    doRefreshPreview();

    logMsg("Engineering canvas example ready");

    g_window->onClose([]() {
        saveSettings();

        if (g_tree) {
            delete g_tree;
            g_tree = nullptr;
        }
        if (g_canvas) {
            delete g_canvas;
            g_canvas = nullptr;
        }
        if (g_settings) {
            delete g_settings;
            g_settings = nullptr;
        }
    });
}

void loop() {
}
