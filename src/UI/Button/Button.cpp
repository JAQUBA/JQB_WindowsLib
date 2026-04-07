// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

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
                bool handled = false;
                // Jeśli długie naciśnięcie nie zostało już wykryte
                if (!pressIt->second.longPressTriggered) {
                    auto currentTime = std::chrono::steady_clock::now();
                    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                        currentTime - pressIt->second.pressTime).count();
                    
                    // Wywołaj callback krótkiego naciśnięcia tylko jeśli czas był krótki
                    if (elapsedMs < LONG_PRESS_DURATION_MS) {
                        button->handleClick();
                        handled = true;
                    }
                } else {
                    handled = true;  // Długie naciśnięcie już obsłużone
                }
                
                // Usuń informację o naciśnięciu
                pressedButtons.erase(pressIt);

                if (handled) {
                    // Zwolnij capture i odtwórz stan wizualny, ale NIE deleguj
                    // do defaultButtonProc (żeby nie wygenerował BN_CLICKED)
                    ReleaseCapture();
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                }
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

        // Hover tracking for owner-draw buttons
        case WM_MOUSEMOVE: {
            if (button && button->m_hasCustomColors && !button->m_isHovered) {
                button->m_isHovered = true;
                InvalidateRect(hwnd, NULL, FALSE);
                TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
                TrackMouseEvent(&tme);
            }
            break;
        }
        case WM_MOUSELEAVE: {
            if (button && button->m_hasCustomColors && button->m_isHovered) {
                button->m_isHovered = false;
                InvalidateRect(hwnd, NULL, FALSE);
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
    if (m_hFont) { DeleteObject(m_hFont); m_hFont = NULL; }
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

    DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP;
    if (m_hasCustomColors) {
        style |= BS_OWNERDRAW;
    } else {
        style |= BS_PUSHBUTTON;
    }
    
    m_hwnd = CreateWindowW(
        L"BUTTON",
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

// Owner-draw styling methods

void Button::setBackColor(COLORREF color) {
    m_backColor = color;
    m_hasCustomColors = true;
    if (m_hwnd) {
        LONG style = GetWindowLongW(m_hwnd, GWL_STYLE);
        if (!(style & BS_OWNERDRAW)) {
            style = (style & ~0x0FL) | BS_OWNERDRAW;
            SetWindowLongW(m_hwnd, GWL_STYLE, style);
        }
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void Button::setTextColor(COLORREF color) {
    m_textColor = color;
    m_hasCustomColors = true;
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

void Button::setHoverColor(COLORREF color) {
    m_hoverColor = color;
}

void Button::setFont(const wchar_t* fontName, int size, bool bold) {
    if (m_hFont) DeleteObject(m_hFont);
    m_hFont = CreateFontW(
        -size, 0, 0, 0,
        bold ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        fontName
    );
    if (m_hwnd) {
        if (!m_hasCustomColors) {
            SendMessageW(m_hwnd, WM_SETFONT, (WPARAM)m_hFont, TRUE);
        } else {
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }
}

void Button::drawOwnerDraw(DRAWITEMSTRUCT* dis) {
    int savedDC = SaveDC(dis->hDC);

    bool pressed = (dis->itemState & ODS_SELECTED) != 0;
    bool focused = (dis->itemState & ODS_FOCUS) != 0;
    bool disabled = (dis->itemState & ODS_DISABLED) != 0;

    // Determine background color
    COLORREF bg;
    if (disabled) {
        bg = RGB(60, 60, 70);
    } else if (pressed && m_backColor != CLR_INVALID) {
        bg = RGB(GetRValue(m_backColor) * 3 / 4,
                 GetGValue(m_backColor) * 3 / 4,
                 GetBValue(m_backColor) * 3 / 4);
    } else if (m_isHovered && m_hoverColor != CLR_INVALID) {
        bg = m_hoverColor;
    } else if (m_backColor != CLR_INVALID) {
        bg = m_backColor;
    } else {
        bg = GetSysColor(COLOR_BTNFACE);
    }

    // Fill rounded rectangle
    HBRUSH hBrush = CreateSolidBrush(bg);
    HPEN hPen = CreatePen(PS_SOLID, 1, bg);
    HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, hBrush);
    HPEN oldPen = (HPEN)SelectObject(dis->hDC, hPen);
    RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
              dis->rcItem.right, dis->rcItem.bottom, 8, 8);
    SelectObject(dis->hDC, oldBrush);
    SelectObject(dis->hDC, oldPen);
    DeleteObject(hBrush);
    DeleteObject(hPen);

    // Select font
    if (m_hFont) SelectObject(dis->hDC, m_hFont);

    // Draw text
    COLORREF fg = disabled ? RGB(100, 100, 110)
                 : (m_textColor != CLR_INVALID ? m_textColor
                    : GetSysColor(COLOR_BTNTEXT));
    SetTextColor(dis->hDC, fg);
    SetBkMode(dis->hDC, TRANSPARENT);

    wchar_t text[256];
    GetWindowTextW(dis->hwndItem, text, 256);
    DrawTextW(dis->hDC, text, -1, &dis->rcItem,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Focus indicator
    if (focused && !disabled) {
        RECT rc = dis->rcItem;
        InflateRect(&rc, -3, -3);
        DrawFocusRect(dis->hDC, &rc);
    }

    RestoreDC(dis->hDC, savedDC);
}