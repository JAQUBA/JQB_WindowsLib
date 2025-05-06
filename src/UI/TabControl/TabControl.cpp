#include "TabControl.h"
#include "../../Util/StringUtils.h"
#include <CommCtrl.h>
#include <map>

// Inicjalizacja zmiennej statycznej
int TabControl::s_nextId = 7000;

// Mapa do przechowywania wskaźników na obiekty TabControl według ich HWND
static std::map<HWND, TabControl*> tabControlsByHwnd;

TabControl::TabControl(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), 
      m_hwnd(NULL), m_onTabChangeCallback(nullptr) {
    m_id = s_nextId++;
}

TabControl::~TabControl() {
    if (m_hwnd) {
        // Usuń wszystkie strony zakładek
        for (HWND tabPage : m_tabPages) {
            DestroyWindow(tabPage);
        }
        m_tabPages.clear();
        
        // Usuń kontrolkę z mapy i zniszcz ją
        tabControlsByHwnd.erase(m_hwnd);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void TabControl::create(HWND parent) {
    // Upewnij się, że kontrolki Common Controls są zainicjowane
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icc);
    
    // Utworzenie kontrolki zakładek
    m_hwnd = CreateWindowExW(
        0,                          // Rozszerzony styl okna
        WC_TABCONTROLW,             // Nazwa klasy
        NULL,                       // Tekst
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,   // Styl okna
        m_x, m_y,                   // Pozycja
        m_width, m_height,          // Rozmiar
        parent,                     // Uchwyt rodzica
        (HMENU)(INT_PTR)m_id,       // ID menu
        _core.hInstance,            // Instancja
        NULL                        // Dane użytkownika
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć kontrolki z zakładkami!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Dodaj kontrolkę do mapy
    tabControlsByHwnd[m_hwnd] = this;
    
    // Ustaw czcionkę systemową
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hFont) {
        SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    }
}

int TabControl::addTab(const char* title) {
    if (!m_hwnd) {
        return -1;
    }
    
    // Konwersja tytułu z UTF-8 na UTF-16
    std::wstring wideTitle = StringUtils::utf8ToWide(title);
    
    // Przygotuj strukturę zakładki
    TCITEMW tci = {0};
    tci.mask = TCIF_TEXT;
    tci.pszText = const_cast<LPWSTR>(wideTitle.c_str());
    
    // Dodaj zakładkę do kontrolki
    int index = TabCtrl_InsertItem(m_hwnd, m_tabPages.size(), &tci);
    if (index < 0) {
        return -1;
    }
    
    // Określ położenie i rozmiar strony zakładki
    RECT rcClient;
    GetClientRect(m_hwnd, &rcClient);
    TabCtrl_AdjustRect(m_hwnd, FALSE, &rcClient);
    
    // Utwórz panel dla zawartości zakładki
    HWND hTabPage = CreateWindowExW(
        0,                          // Rozszerzony styl okna
        L"STATIC",                  // Nazwa klasy
        NULL,                       // Tekst
        WS_CHILD | WS_VISIBLE | SS_WHITERECT, // Styl okna (biały)
        rcClient.left, rcClient.top,           // Pozycja
        rcClient.right - rcClient.left,        // Szerokość
        rcClient.bottom - rcClient.top,        // Wysokość
        m_hwnd,                     // Uchwyt rodzica
        NULL,                       // ID menu
        _core.hInstance,            // Instancja
        NULL                        // Dane użytkownika
    );
    
    if (hTabPage == NULL) {
        // Usuń zakładkę w przypadku błędu
        TabCtrl_DeleteItem(m_hwnd, index);
        return -1;
    }
    
    // Dodaj stronę do wektora
    m_tabPages.push_back(hTabPage);
    
    // Pokaż tylko aktywną stronę
    int selectedTab = getSelectedTab();
    for (int i = 0; i < (int)m_tabPages.size(); ++i) {
        ShowWindow(m_tabPages[i], i == selectedTab ? SW_SHOW : SW_HIDE);
    }
    
    return index;
}

HWND TabControl::getTabPage(int index) const {
    if (index >= 0 && index < (int)m_tabPages.size()) {
        return m_tabPages[index];
    }
    return NULL;
}

void TabControl::removeTab(int index) {
    if (!m_hwnd || index < 0 || index >= (int)m_tabPages.size()) {
        return;
    }
    
    // Zniszcz stronę zakładki
    DestroyWindow(m_tabPages[index]);
    
    // Usuń z wektora
    m_tabPages.erase(m_tabPages.begin() + index);
    
    // Usuń z kontrolki
    TabCtrl_DeleteItem(m_hwnd, index);
    
    // Jeśli usunięto aktywną zakładkę, uaktywnij inną
    int selectedTab = getSelectedTab();
    if (selectedTab >= (int)m_tabPages.size()) {
        selectedTab = m_tabPages.empty() ? -1 : m_tabPages.size() - 1;
        if (selectedTab >= 0) {
            selectTab(selectedTab);
        }
    } else {
        // Odśwież widok zakładek
        for (int i = 0; i < (int)m_tabPages.size(); ++i) {
            ShowWindow(m_tabPages[i], i == selectedTab ? SW_SHOW : SW_HIDE);
        }
    }
}

void TabControl::selectTab(int index) {
    if (!m_hwnd || index < 0 || index >= (int)m_tabPages.size()) {
        return;
    }
    
    // Wybierz zakładkę
    TabCtrl_SetCurSel(m_hwnd, index);
    
    // Ukryj wszystkie strony oprócz wybranej
    for (int i = 0; i < (int)m_tabPages.size(); ++i) {
        ShowWindow(m_tabPages[i], i == index ? SW_SHOW : SW_HIDE);
    }
    
    // Wywołaj callback, jeśli istnieje
    if (m_onTabChangeCallback) {
        m_onTabChangeCallback(index);
    }
}

int TabControl::getSelectedTab() const {
    if (m_hwnd) {
        return TabCtrl_GetCurSel(m_hwnd);
    }
    return -1;
}

void TabControl::setTabTitle(int index, const char* title) {
    if (!m_hwnd || index < 0 || index >= (int)m_tabPages.size()) {
        return;
    }
    
    // Konwersja tytułu z UTF-8 na UTF-16
    std::wstring wideTitle = StringUtils::utf8ToWide(title);
    
    // Przygotuj strukturę zakładki
    TCITEMW tci = {0};
    tci.mask = TCIF_TEXT;
    tci.pszText = const_cast<LPWSTR>(wideTitle.c_str());
    
    // Zaktualizuj tekst zakładki
    TabCtrl_SetItem(m_hwnd, index, &tci);
}

std::string TabControl::getTabTitle(int index) const {
    if (!m_hwnd || index < 0 || index >= (int)m_tabPages.size()) {
        return "";
    }
    
    // Pobierz długość tekstu
    wchar_t buffer[256];
    TCITEMW tci = {0};
    tci.mask = TCIF_TEXT;
    tci.pszText = buffer;
    tci.cchTextMax = sizeof(buffer) / sizeof(buffer[0]);
    
    if (!TabCtrl_GetItem(m_hwnd, index, &tci)) {
        return "";
    }
    
    // Konwersja z UTF-16 na UTF-8
    return StringUtils::wideToUtf8(buffer);
}

int TabControl::getTabCount() const {
    if (m_hwnd) {
        return TabCtrl_GetItemCount(m_hwnd);
    }
    return (int)m_tabPages.size();
}

void TabControl::onTabChange(std::function<void(int)> callback) {
    m_onTabChangeCallback = callback;
}

void TabControl::handleSelection() {
    int selectedTab = getSelectedTab();
    if (selectedTab >= 0) {
        // Ukryj wszystkie strony oprócz wybranej
        for (int i = 0; i < (int)m_tabPages.size(); ++i) {
            ShowWindow(m_tabPages[i], i == selectedTab ? SW_SHOW : SW_HIDE);
        }
        
        // Wywołaj callback, jeśli istnieje
        if (m_onTabChangeCallback) {
            m_onTabChangeCallback(selectedTab);
        }
    }
}