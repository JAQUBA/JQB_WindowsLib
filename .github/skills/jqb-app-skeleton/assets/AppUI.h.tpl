#pragma once

#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <UI/TextArea/TextArea.h>
#include <UI/Theme/Theme.h>

class AppState;

class AppUI {
public:
    AppUI();
    void create(AppState& state);
    void onLoop();

private:
    AppState* m_state;
    SimpleWindow* m_window;
    TextArea* m_logArea;
    Button* m_connectBtn;
};
