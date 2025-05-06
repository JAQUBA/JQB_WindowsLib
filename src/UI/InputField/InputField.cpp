#include "InputField.h"
#include "../../Util/StringUtils.h"
#include <map>
#include <windows.h>
#include <commctrl.h>

// Jeśli definicja EM_SETCUEBANNER nie jest dostępna, zdefiniuj ją
#ifndef EM_SETCUEBANNER
#define EM_SETCUEBANNER (ECM_FIRST + 1)
#endif

// Inicjalizacja zmiennej statycznej
int InputField::s_nextId = 2000;

// Mapa do przechowywania wskaźników na obiekty InputField według ich HWND
static std::map<HWND, InputField*> inputFieldsByHwnd;

// Oryginalna procedura okna dla pola edycji
static WNDPROC defaultEditProc = nullptr;

// Nowa procedura okna dla pola edycji, obsługuje zdarzenia
LRESULT CALLBACK CustomEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    InputField* inputField = nullptr;
    
    // Pobierz wskaźnik do pola edycji
    auto it = inputFieldsByHwnd.find(hwnd);
    if (it != inputFieldsByHwnd.end()) {
        inputField = it->second;
    } else {
        // Jeśli nie znaleziono pola, wywołaj domyślną procedurę
        return CallWindowProc(defaultEditProc, hwnd, uMsg, wParam, lParam);
    }
    
    switch (uMsg) {
        case WM_CHAR:
            // Obsługa wpisywania znaków
            if (wParam == VK_RETURN) {
                // Obsługa naciśnięcia klawisza Enter
                inputField->handleTextChange();
                return 0;
            }
            break;
        
        case WM_COMMAND:
            // Obsługa zmian w polu edycji (gdy wygenerowane przez kontrolkę)
            if (HIWORD(wParam) == EN_CHANGE) {
                inputField->handleTextChange();
            }
            break;
    }
    
    // Wywołanie oryginalnej procedury okna dla standardowej obsługi
    return CallWindowProc(defaultEditProc, hwnd, uMsg, wParam, lParam);
}

InputField::InputField(int x, int y, int width, int height, const char* defaultText,
                       std::function<void(InputField*, const char*)> onTextChange)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(defaultText),
      m_maxLength(0), m_readOnly(false), m_isPassword(false), m_hwnd(NULL), 
      m_onTextChangeCallback(onTextChange) {
    m_id = s_nextId++;
}

InputField::~InputField() {
    if (m_hwnd) {
        // Usuń pole z mapy
        inputFieldsByHwnd.erase(m_hwnd);
        
        // Zniszcz okno pola edycji
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void InputField::create(HWND parent) {
    // Określenie stylu dla pola edycji
    DWORD style = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL;
    
    if (m_readOnly) {
        style |= ES_READONLY;
    }
    
    if (m_isPassword) {
        style |= ES_PASSWORD;
    }
    
    // Konwersja tekstu z UTF-8 na UTF-16 dla kompatybilności z Windows API
    std::wstring wideText = StringUtils::utf8ToWide(m_text);
    
    m_hwnd = CreateWindowW(
        L"EDIT",
        wideText.c_str(),
        style,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć pola wprowadzania!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Ustaw limit znaków, jeśli został określony
    if (m_maxLength > 0) {
        SendMessage(m_hwnd, EM_LIMITTEXT, m_maxLength, 0);
    }
    
    // Dodaj pole do mapy
    inputFieldsByHwnd[m_hwnd] = this;
    
    // Podklasowanie procedury okna pola edycji
    if (!defaultEditProc) {
        defaultEditProc = (WNDPROC)GetWindowLongPtr(m_hwnd, GWLP_WNDPROC);
    }
    
    // Ustaw naszą własną procedurę
    SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)CustomEditProc);
    
    // Ustaw czcionkę systemową
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    if (hFont) {
        SendMessage(m_hwnd, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
    }
    
    // Ustaw tekst zastępczy (placeholder), jeśli istnieje
    if (!m_placeholder.empty() && m_text.empty()) {
        std::wstring widePlaceholder = StringUtils::utf8ToWide(m_placeholder);
        // Używamy SendMessage z EM_SETCUEBANNER do ustawienia tekstu zastępczego
        SendMessageW(m_hwnd, EM_SETCUEBANNER, TRUE, (LPARAM)widePlaceholder.c_str());
    }
}

void InputField::setText(const char* text) {
    m_text = text;
    if (m_hwnd) {
        // Konwersja tekstu z UTF-8 na UTF-16
        std::wstring wideText = StringUtils::utf8ToWide(m_text);
        SetWindowTextW(m_hwnd, wideText.c_str());
    }
}

std::string InputField::getText() const {
    if (m_hwnd) {
        // Pobierz długość tekstu
        int length = GetWindowTextLengthW(m_hwnd) + 1;
        std::wstring wideBuffer(length, L'\0');
        
        // Pobierz tekst z kontrolki
        GetWindowTextW(m_hwnd, &wideBuffer[0], length);
        
        // Konwersja z UTF-16 na UTF-8
        return StringUtils::wideToUtf8(wideBuffer.c_str());
    }
    return m_text;
}

void InputField::setPlaceholder(const char* placeholderText) {
    m_placeholder = placeholderText;
    if (m_hwnd && m_text.empty()) {
        std::wstring widePlaceholder = StringUtils::utf8ToWide(m_placeholder);
        // Używamy SendMessage z EM_SETCUEBANNER do ustawienia tekstu zastępczego
        SendMessageW(m_hwnd, EM_SETCUEBANNER, TRUE, (LPARAM)widePlaceholder.c_str());
    }
}

void InputField::setMaxLength(int maxLength) {
    m_maxLength = maxLength;
    if (m_hwnd && m_maxLength > 0) {
        SendMessage(m_hwnd, EM_LIMITTEXT, m_maxLength, 0);
    }
}

void InputField::setReadOnly(bool readOnly) {
    m_readOnly = readOnly;
    if (m_hwnd) {
        SendMessage(m_hwnd, EM_SETREADONLY, m_readOnly ? TRUE : FALSE, 0);
    }
}

void InputField::setPassword(bool isPassword) {
    m_isPassword = isPassword;
    if (m_hwnd) {
        // Zmiana typu pola na hasło lub zwykły tekst
        DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
        if (m_isPassword) {
            style |= ES_PASSWORD;
        } else {
            style &= ~ES_PASSWORD;
        }
        SetWindowLong(m_hwnd, GWL_STYLE, style);
        
        // Odświeżenie wyglądu kontrolki
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void InputField::handleTextChange() {
    if (m_onTextChangeCallback) {
        std::string currentText = getText();
        m_onTextChangeCallback(this, currentText.c_str());
    }
}