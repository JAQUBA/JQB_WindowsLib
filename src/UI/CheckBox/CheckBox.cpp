#include "CheckBox.h"
#include "../../Util/StringUtils.h"
#include <map>
#include <windows.h>

// Inicjalizacja zmiennej statycznej
int CheckBox::s_nextId = 3000;

// Mapa do przechowywania wskaźników na obiekty CheckBox według ich HWND
static std::map<HWND, CheckBox*> checkBoxesByHwnd;

CheckBox::CheckBox(int x, int y, int width, int height, const char* text, bool checked,
                   std::function<void(CheckBox*, bool)> onChange)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), 
      m_checked(checked), m_hwnd(NULL), m_onChangeCallback(onChange) {
    m_id = s_nextId++;
}

CheckBox::~CheckBox() {
    if (m_hwnd) {
        // Usuń pole wyboru z mapy
        checkBoxesByHwnd.erase(m_hwnd);
        
        // Zniszcz okno
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void CheckBox::create(HWND parent) {
    // Konwersja tekstu z UTF-8 na UTF-16 dla kompatybilności z Windows API
    std::wstring wideText = StringUtils::utf8ToWide(m_text);
    
    m_hwnd = CreateWindowW(
        L"BUTTON",
        wideText.c_str(),
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć pola wyboru!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Dodaj pole wyboru do mapy
    checkBoxesByHwnd[m_hwnd] = this;
    
    // Ustaw stan początkowy
    SendMessage(m_hwnd, BM_SETCHECK, m_checked ? BST_CHECKED : BST_UNCHECKED, 0);
    
    // Ustaw czcionkę systemową
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hFont) {
        SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    }
}

void CheckBox::setText(const char* text) {
    m_text = text;
    if (m_hwnd) {
        // Konwersja tekstu z UTF-8 na UTF-16
        std::wstring wideText = StringUtils::utf8ToWide(m_text);
        SetWindowTextW(m_hwnd, wideText.c_str());
    }
}

std::string CheckBox::getText() const {
    return m_text;
}

void CheckBox::setChecked(bool checked) {
    m_checked = checked;
    if (m_hwnd) {
        SendMessage(m_hwnd, BM_SETCHECK, m_checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool CheckBox::isChecked() const {
    if (m_hwnd) {
        LRESULT state = SendMessage(m_hwnd, BM_GETCHECK, 0, 0);
        return (state == BST_CHECKED);
    }
    return m_checked;
}

void CheckBox::handleClick() {
    // Stan powinien już być zmieniony w odpowiedzi na kliknięcie przez system
    m_checked = (SendMessage(m_hwnd, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    // Wywołaj handleStateChange
    handleStateChange();
}

void CheckBox::handleStateChange() {
    if (m_onChangeCallback) {
        m_onChangeCallback(this, m_checked);
    }
}