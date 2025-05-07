#include "SimpleWindow.h"
#include "../Button/Button.h"
#include "../Label/Label.h"
#include "../Select/Select.h"
#include "../TextArea/TextArea.h"
#include "../ValueDisplay/ValueDisplay.h"
#include "../Chart/Chart.h"
#include "../../Util/StringUtils.h"
#include <CommCtrl.h>  // Dodany plik nagłówkowy dla TCN_SELCHANGE

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
    mainWindow.lpszClassName = L"OWON_OW18B_Window";
    mainWindow.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mainWindow.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    HICON hIcon = LoadIcon(_core.hInstance, MAKEINTRESOURCE(m_iconId));
    if (hIcon) {
        mainWindow.hIcon = hIcon;
    } else {
        mainWindow.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    // Wyrejestruj klasę, jeśli już istnieje (również wersja W)
    UnregisterClassW(L"OWON_OW18B_Window", _core.hInstance);
    
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
        L"OWON_OW18B_Window",
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
        case WM_COMMAND: {
            int controlId = LOWORD(wParam);
            int notificationCode = HIWORD(wParam);
            
            if (s_instance) {
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
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        default:
            return DefWindowProcW(hwnd, uMsg, wParam, lParam); // Zmiana na DefWindowProcW
    }  // Zamknięcie switch
}  // Zamknięcie funkcji WindowProc
