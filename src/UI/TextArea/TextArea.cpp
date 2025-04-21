#include "TextArea.h"

// Inicjalizacja statycznej zmiennej
int TextArea::s_nextId = 4000;

TextArea::TextArea(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_hwnd(NULL) {
    m_id = s_nextId++;
}

TextArea::~TextArea() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void TextArea::create(HWND parent) {
    m_hwnd = CreateWindow(
        TEXT("EDIT"),
        m_text.c_str(),
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );

    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć kontrolki TextArea!", L"Błąd", MB_ICONERROR);
        return;
    }

    // Ustawienie czcionki (opcjonalnie)
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hFont) {
        SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    }
}

void TextArea::setText(const char* text) {
    m_text = text;
    if (m_hwnd) {
        SetWindowText(m_hwnd, m_text.c_str());
    }
}

void TextArea::append(const std::string& text) {
    if (!m_hwnd) return;
    
    // Pobierz aktualną długość tekstu
    int textLength = GetWindowTextLength(m_hwnd);
    
    // Ustaw kursor na końcu tekstu
    SendMessage(m_hwnd, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);
    
    // Dodaj znak nowej linii, jeśli tekst nie jest pusty i ostatni znak nie jest już nową linią
    if (textLength > 0 && m_text[m_text.length() - 1] != '\n') {
        SendMessage(m_hwnd, EM_REPLACESEL, TRUE, (LPARAM)"\r\n");
        m_text += "\r\n";
    }
    
    // Dodaj nowy tekst
    SendMessage(m_hwnd, EM_REPLACESEL, TRUE, (LPARAM)text.c_str());
    m_text += text;
    
    // Przewiń do końca
    SendMessage(m_hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void TextArea::clear() {
    m_text.clear();
    if (m_hwnd) {
        SetWindowText(m_hwnd, "");
    }
}