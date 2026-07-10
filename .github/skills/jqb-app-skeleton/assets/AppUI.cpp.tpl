#include <Core.h>
#include "AppUI.h"
#include "AppState.h"

AppUI::AppUI() : m_state(NULL), m_window(NULL), m_logArea(NULL), m_connectBtn(NULL) {}

void AppUI::create(AppState& state) {
    m_state = &state;

    m_window = new SimpleWindow(1024, 700, "New JQB App", 0);
    m_window->init();

    Theme theme = Theme::catppuccinMocha();
    applyTheme(m_window, theme);

    m_window->add(new Label(24, 20, 300, 26, L"Connection"));

    m_connectBtn = new Button(24, 56, 160, 34, "Connect", [this](Button*) {
        m_state->connected = true;
        m_state->logger.info(L"Connected");
    });
    m_window->add(m_connectBtn);
    styleAccentButton(m_connectBtn, theme);

    m_logArea = new TextArea(24, 110, 976, 540);
    m_window->add(m_logArea);
    m_state->logger.attach(m_logArea);
}

void AppUI::onLoop() {
}
