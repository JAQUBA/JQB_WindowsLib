#include "Button.h"
#include "../../Util/StringUtils.h"
#include <chrono>
#include <map>
#include <windows.h>

// Inicjalizacja statycznej zmiennej
int Button::s_nextId = 1000;

// Mapa przechowująca wszystkie instancje przycisków według ich HWND
static std::map<HWND, Button*> buttonsByHwnd;

// Mapa przycisków według ID
static std::map<int, Button*> buttonsById;

// Długość długiego naciśnięcia w milisekundach
static const int LONG_PRESS_DURATION_MS = 800;

// Struktura przechowująca informacje o naciśnięciu przycisku
struct ButtonPress {
    std::chrono::steady_clock::time_point pressTime;
    bool longPressTriggered;
};

// Mapa przechowująca informacje o naciśniętych przyciskach
static std::map<HWND, ButtonPress> pressedButtons;
// Mapa przechowująca informacje o naciśniętych przyciskach (wg ID)
static std::map<int, ButtonPress> pressedButtonsById;

// Oryginalna procedura okna przycisku Windows
static WNDPROC defaultButtonProc = nullptr;

// Nowa procedura okna dla przycisków
LRESULT CALLBACK CustomButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Button* button = nullptr;
    
    // Pobierz wskaźnik do przycisku
    auto it = buttonsByHwnd.find(hwnd);
    if (it != buttonsByHwnd.end()) {
        button = it->second;
    } else {
        // Jeśli nie znaleziono przycisku, wywołaj domyślną procedurę
        return CallWindowProc(defaultButtonProc, hwnd, uMsg, wParam, lParam);
    }
    
    switch (uMsg) {
        case WM_LBUTTONDOWN: {
            // Zapisz czas naciśnięcia przycisku
            ButtonPress info;
            info.pressTime = std::chrono::steady_clock::now();
            info.longPressTriggered = false;
            pressedButtons[hwnd] = info;
            
            // Ustaw timer, który będzie sprawdzał długie naciśnięcia
            SetTimer(hwnd, 1000, 50, NULL); // Co 50ms
            
            // NIE wywołujemy już callbacku dla krótkiego naciśnięcia tutaj
            // Będzie on wywołany tylko po zwolnieniu przycisku, jeśli czas naciśnięcia był krótki
            
            break;
        }
        
        case WM_LBUTTONUP: {
            // Usuń timer
            KillTimer(hwnd, 1000);
            
            // Sprawdź, czy naciśnięcie było krótkie czy długie
            auto pressIt = pressedButtons.find(hwnd);
            if (pressIt != pressedButtons.end()) {
                // Jeśli długie naciśnięcie nie zostało już wykryte
                if (!pressIt->second.longPressTriggered) {
                    auto currentTime = std::chrono::steady_clock::now();
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        currentTime - pressIt->second.pressTime).count();
                    
                    // Wywołaj callback krótkiego naciśnięcia tylko jeśli czas był krótki
                    if (elapsedMs < LONG_PRESS_DURATION_MS) {
                        button->handleClick();
                    }
                }
                
                // Usuń informację o naciśnięciu
                pressedButtons.erase(pressIt);
            }
            
            break;
        }
        
        case WM_TIMER: {
            if (wParam == 1000) { // Nasz timer do długich naciśnięć
                auto pressIt = pressedButtons.find(hwnd);
                if (pressIt != pressedButtons.end() && !pressIt->second.longPressTriggered) {
                    auto currentTime = std::chrono::steady_clock::now();
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        currentTime - pressIt->second.pressTime).count();
                    
                    // Jeśli upłynął czas potrzebny do długiego naciśnięcia
                    if (elapsedMs >= LONG_PRESS_DURATION_MS && button->m_onLongClick) {
                        // Oznacz długie naciśnięcie jako wykryte
                        pressIt->second.longPressTriggered = true;
                        
                        // Wywołaj callback dla długiego naciśnięcia
                        button->handleLongClick();
                        
                        // Zatrzymaj timer
                        KillTimer(hwnd, 1000);
                    }
                }
            }
            break;
        }
    }
    
    // Wywołaj oryginalną procedurę okna dla standardowej obsługi
    return CallWindowProc(defaultButtonProc, hwnd, uMsg, wParam, lParam);
}

Button::Button(int x, int y, int width, int height, const char* text, 
               std::function<void(Button*)> onClick,
               std::function<void(Button*)> onLongClick)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_text(text), 
      m_hwnd(NULL), m_onClick(onClick), m_onLongClick(onLongClick) {
    m_id = s_nextId++;
    
    // Dodaj przycisk do mapy według ID
    buttonsById[m_id] = this;
}

Button::~Button() {
    if (m_hwnd) {
        // Usuń przycisk z map
        buttonsByHwnd.erase(m_hwnd);
        buttonsById.erase(m_id);
        
        // Usuń informacje o naciśnięciu, jeśli istnieją
        pressedButtons.erase(m_hwnd);
        pressedButtonsById.erase(m_id);
        
        // Zniszcz okno przycisku
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void Button::create(HWND parent) {
    // Konwersja tekstu z UTF-8 na UTF-16 dla kompatybilności z Windows API
    std::wstring wideText = StringUtils::utf8ToWide(m_text);
    
    m_hwnd = CreateWindowW(
        L"BUTTON",
        wideText.c_str(),
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
        return;
    }
    
    // Dodaj przycisk do mapy
    buttonsByHwnd[m_hwnd] = this;
    
    // Podklasowanie procedury okna przycisku
    if (!defaultButtonProc) {
        defaultButtonProc = (WNDPROC)GetWindowLongPtr(m_hwnd, GWLP_WNDPROC);
    }
    
    // Ustaw naszą własną procedurę
    SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, (LONG_PTR)CustomButtonProc);
}

void Button::handleClick() {
    if (m_onClick) {
        m_onClick(this);
    }
}

void Button::handleLongClick() {
    if (m_onLongClick) {
        m_onLongClick(this);
    }
}

// Implementacja funkcji pomocniczych deklarowanych w Button.h

// Funkcja do rozpoczęcia śledzenia naciśnięcia przycisku według ID
void startButtonPress(int buttonId) {
    auto button = buttonsById.find(buttonId);
    if (button != buttonsById.end()) {
        ButtonPress info;
        info.pressTime = std::chrono::steady_clock::now();
        info.longPressTriggered = false;
        pressedButtonsById[buttonId] = info;
        
        // NIE wywołujemy już callbacku dla krótkiego naciśnięcia tutaj
    }
}

// Funkcja do zakończenia śledzenia naciśnięcia przycisku według ID
void endButtonPress(int buttonId) {
    auto pressIt = pressedButtonsById.find(buttonId);
    if (pressIt != pressedButtonsById.end()) {
        // Jeśli długie naciśnięcie nie zostało wykryte
        if (!pressIt->second.longPressTriggered) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                currentTime - pressIt->second.pressTime).count();
            
            // Wywołaj callback krótkiego naciśnięcia tylko jeśli czas był krótki
            if (elapsedMs < LONG_PRESS_DURATION_MS) {
                auto button = buttonsById.find(buttonId);
                if (button != buttonsById.end()) {
                    button->second->handleClick();
                }
            }
        }
        
        // Usuń informację o naciśnięciu
        pressedButtonsById.erase(pressIt);
    }
}

// Funkcja do sprawdzenia wszystkich naciśniętych przycisków i wykrycia długich naciśnięć
void checkForLongPresses() {
    auto currentTime = std::chrono::steady_clock::now();
    
    // Sprawdzenie naciśnięć według HWND
    for (auto it = pressedButtons.begin(); it != pressedButtons.end(); ) {
        HWND hwnd = it->first;
        ButtonPress& info = it->second;
        
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - info.pressTime).count();
        
        if (elapsedMs >= LONG_PRESS_DURATION_MS && !info.longPressTriggered) {
            auto btnIt = buttonsByHwnd.find(hwnd);
            if (btnIt != buttonsByHwnd.end() && btnIt->second->m_onLongClick) {
                info.longPressTriggered = true;
                btnIt->second->handleLongClick();
            }
        }
        ++it;
    }
    
    // Sprawdzenie naciśnięć według ID
    for (auto it = pressedButtonsById.begin(); it != pressedButtonsById.end(); ) {
        int id = it->first;
        ButtonPress& info = it->second;
        
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime - info.pressTime).count();
        
        if (elapsedMs >= LONG_PRESS_DURATION_MS && !info.longPressTriggered) {
            auto btnIt = buttonsById.find(id);
            if (btnIt != buttonsById.end() && btnIt->second->m_onLongClick) {
                info.longPressTriggered = true;
                btnIt->second->handleLongClick();
            }
        }
        ++it;
    }
}