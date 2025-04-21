#include "Button.h"

// Inicjalizacja statycznej zmiennej
int Button::s_nextId = 1000;

Button::Button(int x, int y, int width, int height, const char* text, std::function<void(Button*)> onClick)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), m_hwnd(NULL), m_onClick(onClick) {
    m_id = s_nextId++;
}

Button::~Button() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void Button::create(HWND parent) {
    m_hwnd = CreateWindow(
        TEXT("BUTTON"),
        m_text.c_str(),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );

    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć przycisku!", L"Błąd", MB_ICONERROR);
    }
}

void Button::handleClick() {
    if (m_onClick) {
        m_onClick(this);
    }
}