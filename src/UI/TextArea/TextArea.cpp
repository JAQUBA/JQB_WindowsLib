#include "TextArea.h"
#include "../../Util/StringUtils.h"
#include <Windows.h>

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

// Konwersja z UTF-8 na Unicode (std::wstring)
std::wstring TextArea::utf8ToWide(const std::string& text) const {
    return StringUtils::utf8ToWide(text);
}

// Konwersja z Unicode na UTF-8
std::string TextArea::wideToUtf8(const std::wstring& wstr) const {
    return StringUtils::wideToUtf8(wstr);
}

std::string TextArea::getTextUTF8() const {
    return wideToUtf8(m_text);
}

void TextArea::create(HWND parent) {
    m_hwnd = CreateWindowW(
        L"EDIT",
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
    setText(std::string(text));
}

void TextArea::setText(const std::string& text) {
    m_text = utf8ToWide(text);
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, m_text.c_str());
    }
}

void TextArea::setText(const wchar_t* text) {
    setText(std::wstring(text));
}

void TextArea::setText(const std::wstring& text) {
    m_text = text;
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, m_text.c_str());
    }
}

void TextArea::append(const std::string& text) {
    append(utf8ToWide(text));
}

void TextArea::append(const std::wstring& text) {
    if (!m_hwnd) return;
    
    // Pobierz aktualną długość tekstu
    int textLength = GetWindowTextLengthW(m_hwnd);
    
    // Ustaw kursor na końcu tekstu
    SendMessage(m_hwnd, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);
    
    // Dodaj nowy tekst bez automatycznego dodawania znaków nowej linii
    SendMessageW(m_hwnd, EM_REPLACESEL, TRUE, (LPARAM)text.c_str());
    m_text += text;
    
    // Przewiń do końca
    SendMessage(m_hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

void TextArea::clear() {
    m_text.clear();
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, L"");
    }
}