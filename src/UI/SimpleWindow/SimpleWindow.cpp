// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#include "SimpleWindow.h"
#include "../Button/Button.h"
#include "../Label/Label.h"
#include "../Select/Select.h"
#include "../TextArea/TextArea.h"
#include "../ValueDisplay/ValueDisplay.h"
#include "../Chart/Chart.h"
#include "../../Util/StringUtils.h"
#include <CommCtrl.h>  // Dodany plik nagłówkowy dla TCN_SELCHANGE

static const wchar_t* WINDOW_CLASS_NAME = L"JQB_SimpleWindow";

// Inicjalizacja statycznej zmiennej
SimpleWindow* SimpleWindow::s_instance = nullptr;

// Identyfikator timera dla długich naciśnięć
static const UINT_PTR LONG_PRESS_TIMER_ID = 1000;

SimpleWindow::SimpleWindow(int width, int height, const char *title, int iconId)
    : m_width(width), m_height(height), m_iconId(iconId), m_hwnd(NULL) {
    // Konwersja tytułu z UTF-8 na UTF-16 i przechowywanie jako wstring
    if (title && title[0] != '\0') {
        m_titleW = StringUtils::utf8ToWide(title);
    } else {
        m_titleW = L"Window";
    }
    s_instance = this;
}

SimpleWindow::~SimpleWindow() {
    // Usunięcie wszystkich komponentów UI z głównego wektora
    for (auto component : m_components) {
        delete component;
    }
    m_components.clear();
    
    // Czyszczę pozostałe wektory bez usuwania obiektów (są już usunięte powyżej)
    m_buttons.clear();
    m_labels.clear();
    m_selects.clear();
    m_textAreas.clear();
    m_valueDisplays.clear();
    m_charts.clear();

    // Usunięcie pędzla tła
    if (m_hBackBrush) { DeleteObject(m_hBackBrush); m_hBackBrush = NULL; }
    if (m_hCtrlBrush) { DeleteObject(m_hCtrlBrush); m_hCtrlBrush = NULL; }

    // Zniszczenie okna
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
    PostQuitMessage(0);
}

bool SimpleWindow::init() {
    // Konsekwentne używanie interfejsu Unicode (W)
    WNDCLASSW mainWindow = {};
    mainWindow.lpfnWndProc = WindowProc;
    mainWindow.hInstance = _core.hInstance;
    mainWindow.lpszClassName = WINDOW_CLASS_NAME;
    mainWindow.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mainWindow.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    HICON hIcon = LoadIcon(_core.hInstance, MAKEINTRESOURCE(m_iconId));
    if (hIcon) {
        mainWindow.hIcon = hIcon;
    } else {
        mainWindow.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    // Wyrejestruj klasę, jeśli już istnieje
    UnregisterClassW(WINDOW_CLASS_NAME, _core.hInstance);
    
    if (!RegisterClassW(&mainWindow)) {
        return false;
    }
    
    // Obliczanie rzeczywistego rozmiaru okna
    RECT windowRect = { 0, 0, m_width, m_height };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
    int actualWidth = windowRect.right - windowRect.left;
    int actualHeight = windowRect.bottom - windowRect.top;
    
    // Tworzenie okna z użyciem funkcji Unicode (CreateWindowW)
    m_hwnd = CreateWindowW(
        WINDOW_CLASS_NAME,
        m_titleW.c_str(),  // Użycie wstring zamiast char*
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        actualWidth, actualHeight,
        NULL,
        NULL,
        _core.hInstance,
        NULL
    );
    
    if (m_hwnd == NULL) {
        MessageBoxW(NULL, L"Nie udało się utworzyć okna", L"Błąd", MB_OK | MB_ICONERROR);
        return false;
    }
    
    ShowWindow(m_hwnd, _core.nCmdShow);
    UpdateWindow(m_hwnd);
    
    return true;
}

void SimpleWindow::close() {
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void SimpleWindow::setMenu(HMENU menu) {
    if (m_hwnd && menu) {
        ::SetMenu(m_hwnd, menu);
        // Przelicz rozmiar okna uwzględniając menu
        RECT rc = { 0, 0, m_width, m_height };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);
        SetWindowPos(m_hwnd, NULL, 0, 0,
                     rc.right - rc.left, rc.bottom - rc.top,
                     SWP_NOMOVE | SWP_NOZORDER);
    }
}

void SimpleWindow::setBackgroundColor(COLORREF color) {
    m_backColor = color;
    if (m_hBackBrush) DeleteObject(m_hBackBrush);
    m_hBackBrush = CreateSolidBrush(color);
    // Create a slightly lighter brush for editable controls
    if (m_hCtrlBrush) DeleteObject(m_hCtrlBrush);
    int r = GetRValue(color), g = GetGValue(color), b = GetBValue(color);
    m_hCtrlBrush = CreateSolidBrush(RGB(
        r + (255 - r) / 8, g + (255 - g) / 8, b + (255 - b) / 8));
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

// Nowa główna metoda do dodawania komponentów UI
void SimpleWindow::add(UIComponent* component) {
    if (component) {
        component->create(m_hwnd);
        m_components.push_back(component);
    }
}

// Metody dla zachowania kompatybilności wstecznej
void SimpleWindow::add(Button* button) {
    add(static_cast<UIComponent*>(button));
    m_buttons.push_back(button);
}

void SimpleWindow::add(Label* label) {
    add(static_cast<UIComponent*>(label));
    m_labels.push_back(label);
}

void SimpleWindow::add(Select* select) {
    add(static_cast<UIComponent*>(select));
    m_selects.push_back(select);
}

void SimpleWindow::add(TextArea* textArea) {
    add(static_cast<UIComponent*>(textArea));
    m_textAreas.push_back(textArea);
}

void SimpleWindow::add(ValueDisplay* valueDisplay) {
    add(static_cast<UIComponent*>(valueDisplay));
    m_valueDisplays.push_back(valueDisplay);
}

void SimpleWindow::add(Chart* chart) {
    add(static_cast<UIComponent*>(chart));
    m_charts.push_back(chart);
}

LRESULT CALLBACK SimpleWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Obsługa wiadomości
    switch (uMsg) {
        // Obsługa kolorów etykiet (Label) z niestandardowymi kolorami
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            if (s_instance) {
                // Label — kolory tekstu i tła
                for (auto* label : s_instance->m_labels) {
                    if (label->getHandle() == hCtrl && label->hasCustomColors()) {
                        if (label->getTextColor() != CLR_INVALID)
                            SetTextColor(hdc, label->getTextColor());
                        if (label->getBackColor() != CLR_INVALID) {
                            SetBkColor(hdc, label->getBackColor());
                            return (LRESULT)label->getBackBrush();
                        }
                        SetBkMode(hdc, TRANSPARENT);
                        // Zwróć pędzel tła okna lub NULL_BRUSH
                        if (s_instance->m_hBackBrush)
                            return (LRESULT)s_instance->m_hBackBrush;
                        return (LRESULT)GetStockObject(NULL_BRUSH);
                    }
                }
                // TextArea (ES_READONLY) — kolory tekstu i tła
                for (auto* ta : s_instance->m_textAreas) {
                    if (ta->getHandle() == hCtrl && ta->hasCustomColors()) {
                        if (ta->getTextColor() != CLR_INVALID)
                            SetTextColor(hdc, ta->getTextColor());
                        if (ta->getBackColor() != CLR_INVALID) {
                            SetBkColor(hdc, ta->getBackColor());
                            return (LRESULT)ta->getBackBrush();
                        }
                        SetBkMode(hdc, TRANSPARENT);
                        if (s_instance->m_hBackBrush)
                            return (LRESULT)s_instance->m_hBackBrush;
                        return (LRESULT)GetStockObject(NULL_BRUSH);
                    }
                }
                // For any non-Label STATIC control, apply window background and text color
                if (s_instance->m_hBackBrush) {
                    if (s_instance->m_textColor != CLR_INVALID)
                        SetTextColor(hdc, s_instance->m_textColor);
                    SetBkMode(hdc, TRANSPARENT);
                    return (LRESULT)s_instance->m_hBackBrush;
                }
            }
            break;
        }

        // Dark background for checkbox/radio controls
        case WM_CTLCOLORBTN: {
            if (s_instance && s_instance->m_hBackBrush) {
                if (s_instance->m_textColor != CLR_INVALID)
                    SetTextColor((HDC)wParam, s_instance->m_textColor);
                return (LRESULT)s_instance->m_hBackBrush;
            }
            break;
        }

        // Dark theme for ComboBox list and edit portion
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
            if (s_instance && s_instance->m_hBackBrush) {
                HDC hdc = (HDC)wParam;
                if (s_instance->m_textColor != CLR_INVALID)
                    SetTextColor(hdc, s_instance->m_textColor);
                if (s_instance->m_backColor != CLR_INVALID)
                    SetBkColor(hdc, s_instance->m_backColor);
                return (LRESULT)s_instance->m_hBackBrush;
            }
            break;
        }

        // Owner-draw controls (buttons)
        case WM_DRAWITEM: {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
            if (dis->CtlType == ODT_BUTTON && s_instance) {
                // Check library Button instances first
                for (auto* btn : s_instance->m_buttons) {
                    if (btn->getId() == (int)dis->CtlID && btn->hasCustomColors()) {
                        btn->drawOwnerDraw(dis);
                        return TRUE;
                    }
                }
                // Check all components via virtual handleDrawItem
                for (auto* comp : s_instance->m_components) {
                    if (comp->handleDrawItem(dis)) {
                        return TRUE;
                    }
                }
                // Then custom callback
                if (s_instance->m_onDrawItem && s_instance->m_onDrawItem(dis)) {
                    return TRUE;
                }
            }
            break;
        }

        // Malowanie tła okna niestandardowym kolorem
        case WM_ERASEBKGND: {
            if (s_instance && s_instance->m_hBackBrush) {
                HDC hdc = (HDC)wParam;
                RECT rc;
                GetClientRect(hwnd, &rc);
                FillRect(hdc, &rc, s_instance->m_hBackBrush);
                return 1;
            }
            break;
        }

        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);
            
            if (s_instance) {
                // Komenda z menu (lParam == 0 oznacza menu, nie kontrolkę)
                if (notificationCode == 0 && lParam == 0 && controlId > 0) {
                    if (s_instance->m_onMenuCommand) {
                        s_instance->m_onMenuCommand(controlId);
                        return 0;
                    }
                }
                
                // Obsługa kliknięć i innych zdarzeń dla wszystkich komponentów
                if (notificationCode == BN_CLICKED) {
                    // Zakończenie naciśnięcia przycisku
                    endButtonPress(controlId);
                    
                    // Wywołanie metody handleClick() odpowiedniego komponentu
                    for (auto component : s_instance->m_components) {
                        if (component->getId() == controlId) {
                            component->handleClick();
                            break;
                        }
                    }
                }
                // Obsługa komunikatów przycisków - naciśnięcie
                else if (notificationCode == BN_PUSHED) {
                    // Rozpoczęcie naciśnięcia przycisku
                    startButtonPress(controlId);
                }
                // Sprawdzenie, czy to zmiana wyboru w kontrolce Select
                else if (notificationCode == CBN_SELCHANGE) {
                    for (auto component : s_instance->m_components) {
                        if (component->getId() == controlId) {
                            component->handleSelection();
                            break;
                        }
                    }
                }
            }
            
            // Jeśli naciśnięto przycisk zamknięcia (ID = 1)
            if (controlId == 1) {
                // Zamknij okno
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
        }
        
        // Dodanie obsługi WM_NOTIFY dla kontrolek takich jak TabControl
        case WM_NOTIFY: {
            NMHDR* nmhdr = (NMHDR*)lParam;
            
            // Obsługa kliknięcia na zakładkę
            if (nmhdr->code == TCN_SELCHANGE) {
                // Znajdź kontrolkę TabControl o tym ID
                if (s_instance) {
                    for (auto component : s_instance->m_components) {
                        if (component->getId() == (int)nmhdr->idFrom) {
                            // Wywołaj handleSelection na znalezionej kontrolce
                            component->handleSelection();
                            break;
                        }
                    }
                }
            }
            return 0;
        }
        
        case WM_TIMER:
            // Obsługa timera dla długich naciśnięć
            if (wParam == LONG_PRESS_TIMER_ID) {
                checkForLongPresses();
            }
            return 0;
            
        case WM_CLOSE:
            // Wywołaj callback onClose przed zamknięciem
            if (s_instance && s_instance->m_onCloseCallback) {
                s_instance->m_onCloseCallback();
            }
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }  // Zamknięcie switch
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}  // Zamknięcie funkcji WindowProc
