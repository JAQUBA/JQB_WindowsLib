#include "Label.h"

// Inicjalizacja statycznej zmiennej
int Label::s_nextId = 2000;

Label::Label(int x, int y, int width, int height, const wchar_t* text)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), m_hwnd(NULL) {
    m_id = s_nextId++;
}

Label::~Label() {
    if (m_hFont) { DeleteObject(m_hFont); m_hFont = NULL; }
    if (m_hBackBrush) { DeleteObject(m_hBackBrush); m_hBackBrush = NULL; }
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
        return;
    }

    // Zastosuj czcionkę jeśli była ustawiona przed create()
    if (m_hFont) {
        SendMessageW(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    }
}

void Label::setText(const wchar_t* text) {
    m_text = text;
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, m_text.c_str());
    }
}

void Label::setFont(const wchar_t* fontName, int size, bool bold, bool italic) {
    if (m_hFont) DeleteObject(m_hFont);
    m_hFont = CreateFontW(
        -size, 0, 0, 0,
        bold ? FW_BOLD : FW_NORMAL,
        italic ? TRUE : FALSE,
        FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        fontName
    );
    if (m_hwnd && m_hFont) {
        SendMessageW(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    }
}

void Label::setTextColor(COLORREF color) {
    m_textColor = color;
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

void Label::setBackColor(COLORREF color) {
    m_backColor = color;
    if (m_hBackBrush) DeleteObject(m_hBackBrush);
    m_hBackBrush = CreateSolidBrush(color);
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

bool Label::hasCustomColors() const {
    return m_textColor != CLR_INVALID || m_backColor != CLR_INVALID;
}