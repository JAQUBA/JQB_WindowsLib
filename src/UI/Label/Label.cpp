#include "Label.h"

// Inicjalizacja statycznej zmiennej
int Label::s_nextId = 2000;

Label::Label(int x, int y, int width, int height, const wchar_t* text)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), m_hwnd(NULL) {
    m_id = s_nextId++;
}

Label::~Label() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void Label::create(HWND parent) {
    m_hwnd = CreateWindowW(
        L"STATIC",
        m_text.c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );

    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć etykiety!", L"Błąd", MB_ICONERROR);
    }
}

void Label::setText(const wchar_t* text) {
    m_text = text;
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, m_text.c_str());
    }
}