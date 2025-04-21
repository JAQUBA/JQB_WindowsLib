#include "Select.h"
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
    // Tworzenie kontrolki combobox
    m_hwnd = CreateWindow(
        TEXT("COMBOBOX"),
        m_text.c_str(),
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS | WS_VSCROLL,
        m_x, m_y,
        m_width, m_height,
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
        SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }

    // Jeśli był wybrany element, ustaw go ponownie
    if (m_selectedIndex >= 0 && m_selectedIndex < (int)m_items.size()) {
        SendMessage(m_hwnd, CB_SETCURSEL, (WPARAM)m_selectedIndex, 0);
    }
    
    // Jeśli istnieją powiązane elementy, zaktualizuj listę
    if (m_linkedItems) {
        updateItems();
    }
}

void Select::handleSelection() {
    int index = (int)SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);
    
    if (index != CB_ERR) {
        m_selectedIndex = index;
        
        // Pobierz tekst wybranego elementu
        char buffer[256];
        SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)buffer);
        m_text = buffer;
        
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
        SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)item);
    }
}

void Select::setText(const char* text) {
    m_text = text;
    if (m_hwnd) {
        SetWindowText(m_hwnd, m_text.c_str());
    }
}

void Select::clear() {
    m_items.clear();
    m_selectedIndex = -1;
    
    if (m_hwnd) {
        SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
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
    SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
    m_items.clear();
    
    // Dodaj wszystkie elementy z powiązanej listy
    for (const auto& item : *m_linkedItems) {
        m_items.push_back(item);
        SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)item.c_str());
    }
    
    // Ustaw pierwszy element jako wybrany
    if (!m_items.empty()) {
        SendMessage(m_hwnd, CB_SETCURSEL, 0, 0);
        m_selectedIndex = 0;
        m_text = m_items[0];
    } else {
        m_selectedIndex = -1;
        m_text = "";
    }
}