#include <Core.h>

#include "AppState.h"
#include "AppUI.h"

void setup() {
    g_window = new SimpleWindow(1240, 760, "Example 04 - Document Editor Preview", 0);
    g_window->init();
    g_window->setBackgroundColor(RGB(34, 37, 46));
    g_window->setTextColor(RGB(226, 230, 239));

    g_settings = new ConfigManager("document_editor_preview.ini");

    createUI(g_window);
    loadSettings();
    doRenderPreview();

    g_window->onClose([]() {
        saveSettings();

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
