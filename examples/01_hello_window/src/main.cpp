#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>

SimpleWindow* g_window = nullptr;
Label* g_label = nullptr;
int g_clicks = 0;

void setup() {
    g_window = new SimpleWindow(500, 260, "Example 01 - Hello Window", 0);
    g_window->init();

    g_window->add(new Label(20, 20, 450, 24, L"JQB_WindowsLib runnable project"));

    g_label = new Label(20, 55, 450, 24, L"Clicks: 0");
    g_window->add(g_label);

    g_window->add(new Button(20, 95, 180, 34, "Click", [](Button*) {
        g_clicks++;
        wchar_t buf[64];
        _snwprintf(buf, 64, L"Clicks: %d", g_clicks);
        if (g_label) g_label->setText(buf);
    }));

    g_window->add(new Button(210, 95, 180, 34, "Reset", [](Button*) {
        g_clicks = 0;
        if (g_label) g_label->setText(L"Clicks: 0");
    }));
}

void loop() {
}
