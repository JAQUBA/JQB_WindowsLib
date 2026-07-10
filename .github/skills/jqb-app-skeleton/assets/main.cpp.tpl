#include <Core.h>
#include "AppState.h"
#include "AppUI.h"

AppState g_app;
AppUI g_ui;

void setup() {
    g_app.load();
    g_ui.create(g_app);
    g_app.afterUiReady();
}

void loop() {
    g_app.tick();
    g_ui.onLoop();
}
