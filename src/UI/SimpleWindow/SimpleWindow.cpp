#include "SimpleWindow.h"
#include "../Button/Button.h"
#include "../Label/Label.h"
#include "../Select/Select.h"
#include "../TextArea/TextArea.h"

// Inicjalizacja statycznej zmiennej
SimpleWindow* SimpleWindow::s_instance = nullptr;

SimpleWindow::SimpleWindow(int width, int height, const char *title, int iconId)
    : m_width(width), m_height(height), m_title(title), m_iconId(iconId), m_hwnd(NULL) {
    s_instance = this;
}

SimpleWindow::~SimpleWindow() {
    // Usunięcie kontrolek
    for (auto button : m_buttons) {
        delete button;
    }
    m_buttons.clear();

    for (auto label : m_labels) {
        delete label;
    }
    m_labels.clear();

    for (auto select : m_selects) {
        delete select;
    }
    m_selects.clear();

    for (auto textArea : m_textAreas) {
        delete textArea;
    }
    m_textAreas.clear();

    // Zniszczenie okna
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

#define CLASS_NAME TEXT("OWON_OW18B_Window")

bool SimpleWindow::init() {
    WNDCLASS mainWindow = {};
    mainWindow.lpfnWndProc = WindowProc;
    mainWindow.hInstance = _core.hInstance;
    mainWindow.lpszClassName = TEXT("OWON_OW18B_Window");
    mainWindow.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    mainWindow.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    HICON hIcon = LoadIcon(_core.hInstance, MAKEINTRESOURCE(m_iconId));
    if (hIcon) {
        mainWindow.hIcon = hIcon;
    } else {
        mainWindow.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    if (!RegisterClass(&mainWindow)) {
        return false;
    }

    m_hwnd = CreateWindow(
        CLASS_NAME,
        m_title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        m_width, m_height,
        NULL,
        NULL,
        _core.hInstance,
        this
    );

    if (m_hwnd == NULL) {
        MessageBox(NULL, TEXT("Nie udało się utworzyć okna"), TEXT("Błąd"), MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(m_hwnd, _core.nCmdShow);
    UpdateWindow(m_hwnd);

    return true;
}

void SimpleWindow::close() {
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

// Metoda dodająca przycisk do okna
void SimpleWindow::add(Button* button) {
    button->create(m_hwnd);
    m_buttons.push_back(button);
}

// Metoda dodająca etykietę do okna
void SimpleWindow::add(Label* label) {
    label->create(m_hwnd);
    m_labels.push_back(label);
}

// Metoda dodająca komponent Select do okna
void SimpleWindow::add(Select* select) {
    select->create(m_hwnd);
    m_selects.push_back(select);
}

// Metoda dodająca komponent TextArea do okna
void SimpleWindow::add(TextArea* textArea) {
    textArea->create(m_hwnd);
    m_textAreas.push_back(textArea);
}

LRESULT CALLBACK SimpleWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Obsługa wiadomości
    switch (uMsg) {
        case WM_COMMAND:
            // Sprawdzenie, czy to kliknięcie przycisku
            if (HIWORD(wParam) == BN_CLICKED) {
                int controlId = LOWORD(wParam);
                // Sprawdź, czy to nasz przycisk
                if (s_instance) {
                    for (auto button : s_instance->m_buttons) {
                        if (button->getId() == controlId) {
                            button->handleClick();
                            return 0;
                        }
                    }
                }
            }
            // Sprawdzenie, czy to zmiana wyboru w kontrolce Select
            else if (HIWORD(wParam) == CBN_SELCHANGE) {
                int controlId = LOWORD(wParam);
                // Sprawdź, czy to nasza kontrolka Select
                if (s_instance) {
                    for (auto select : s_instance->m_selects) {
                        if (select->getId() == controlId) {
                            select->handleSelection();
                            return 0;
                        }
                    }
                }
            }
            // Jeśli naciśnięto przycisk zamknięcia (ID = 1)
            if (LOWORD(wParam) == 1) {
                // Zamknij okno
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            }
            return 0;
            
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}