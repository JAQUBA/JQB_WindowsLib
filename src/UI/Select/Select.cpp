#include "Select.h"
#include "../../Util/StringUtils.h"
#include <CommCtrl.h>

// Inicjalizacja statycznej zmiennej
int Select::s_nextId = 3000;

Select::Select(int x, int y, int width, int height, const char* text, std::function<void(Select*)> onChange)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), m_hwnd(NULL), m_onChange(onChange), 
      m_selectedIndex(-1), m_linkedItems(nullptr) {
    m_id = s_nextId++;
}

Select::~Select() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void Select::create(HWND parent) {
    // Konwersja tekstu z UTF-8 na UTF-16
    std::wstring wideText = StringUtils::utf8ToWide(m_text);
    
    // Tworzenie kontrolki combobox
    m_hwnd = CreateWindowW(
        L"COMBOBOX",
        wideText.c_str(),
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS | WS_VSCROLL,
        m_x, m_y,
        m_width, m_height + 150, // Zwiększona wysokość dla rozwijalnej listy
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );

    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć kontrolki Select!", L"Błąd", MB_ICONERROR);
        return;
    }

    // Dodanie elementów do kontrolki
    for (const auto& item : m_items) {
        std::wstring wideItem = StringUtils::utf8ToWide(item);
        SendMessageW(m_hwnd, CB_ADDSTRING, 0, (LPARAM)wideItem.c_str());
    }

    // Jeśli był wybrany element, ustaw go ponownie
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_items.size()) {
        SendMessageW(m_hwnd, CB_SETCURSEL, (WPARAM)m_selectedIndex, 0);
    }
    
    // Jeśli istnieją powiązane elementy, zaktualizuj listę
    if (m_linkedItems) {
        updateItems();
    }
    
    // Ustaw rzeczywistą wysokość kontrolki edycji (część nie rozwijana)
    SendMessageW(m_hwnd, CB_SETITEMHEIGHT, (WPARAM)-1, (LPARAM)m_height - 6);
    
    // Ustaw wysokość elementów w rozwijanej liście
    SendMessageW(m_hwnd, CB_SETDROPPEDWIDTH, (WPARAM)m_width, 0);
}

void Select::handleSelection() {
    int index = (int)SendMessageW(m_hwnd, CB_GETCURSEL, 0, 0);
    
    if (index != CB_ERR) {
        m_selectedIndex = index;
        
        // Pobierz tekst wybranego elementu
        wchar_t buffer[256];
        SendMessageW(m_hwnd, CB_GETLBTEXT, index, (LPARAM)buffer);
        m_text = StringUtils::wideToUtf8(buffer);
        
        // Wywołaj funkcję callback, jeśli istnieje
        if (m_onChange) {
            m_onChange(this);
        }
    }
}

void Select::addItem(const char* item) {
    m_items.push_back(item);
    
    // Jeśli kontrolka jest już utworzona, dodaj element do listy
    if (m_hwnd) {
        std::wstring wideItem = StringUtils::utf8ToWide(item);
        SendMessageW(m_hwnd, CB_ADDSTRING, 0, (LPARAM)wideItem.c_str());
    }
}

void Select::setText(const char* text) {
    m_text = text;
    if (m_hwnd) {
        std::wstring wideText = StringUtils::utf8ToWide(m_text);
        SetWindowTextW(m_hwnd, wideText.c_str());
    }
}

void Select::clear() {
    m_items.clear();
    m_selectedIndex = -1;
    
    if (m_hwnd) {
        SendMessageW(m_hwnd, CB_RESETCONTENT, 0, 0);
    }
}

void Select::link(const std::vector<std::string>* items) {
    m_linkedItems = items;
    
    // Jeśli kontrolka jest już utworzona, zaktualizuj jej elementy
    if (m_linkedItems && m_hwnd) {
        updateItems();
    }
}

void Select::updateItems() {
    if (!m_linkedItems) return;
    
    // Wyczyść aktualną listę
    SendMessageW(m_hwnd, CB_RESETCONTENT, 0, 0);
    m_items.clear();
    
    // Dodaj wszystkie elementy z powiązanej listy
    for (const auto& item : *m_linkedItems) {
        m_items.push_back(item);
        std::wstring wideItem = StringUtils::utf8ToWide(item);
        SendMessageW(m_hwnd, CB_ADDSTRING, 0, (LPARAM)wideItem.c_str());
    }
    
    // Ustaw pierwszy element jako wybrany
    if (!m_items.empty()) {
        SendMessageW(m_hwnd, CB_SETCURSEL, 0, 0);
        m_selectedIndex = 0;
        m_text = m_items[0];
    } else {
        m_selectedIndex = -1;
        m_text = "";
    }
}