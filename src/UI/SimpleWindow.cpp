#include "SimpleWindow.h"
#include <windows.h>
#include <string>

// Inicjalizacja statycznej zmiennej
SimpleWindow* SimpleWindow::s_instance = nullptr;

SimpleWindow::SimpleWindow(int width, int height, const std::wstring& title, int iconId)
    : m_width(width), m_height(height), m_title(title), m_iconId(iconId), m_hwnd(NULL) {
    // Zapisujemy wskaźnik na instancję do użycia w funkcji obsługującej wiadomości
    s_instance = this;
    
    // Inicjalizacja handlerów
    for (int i = 0; i < WM_APP + 100; i++) {
        m_handlers[i] = nullptr;
    }
}

SimpleWindow::~SimpleWindow() {
    // Upewniamy się, że okno jest zamknięte
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

bool SimpleWindow::init() {
    // Nazwa klasy okna
    const wchar_t CLASS_NAME[] = L"SimpleWindowClass";
    
    // Zarejestruj klasę okna
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(m_iconId));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    
    RegisterClassW(&wc);
    
    // Utwórz okno z bezpośrednim łańcuchem testowym zamiast m_title.c_str()
    m_hwnd = CreateWindowW(
        CLASS_NAME,                  // Nazwa klasy okna
        L"TEST TITLE",               // Stały tytuł testowy
        WS_OVERLAPPEDWINDOW,         // Styl okna
        CW_USEDEFAULT, CW_USEDEFAULT, // Pozycja X i Y
        m_width, m_height,           // Szerokość i wysokość
        NULL,                        // Rodzic
        NULL,                        // Menu
        GetModuleHandle(NULL),       // Instance
        NULL                         // Dodatkowe dane
    );
    
    if (m_hwnd == NULL) {
        return false;
    }
    
    // Po utworzeniu okna, ustaw tytuł bezpośrednio
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, m_title.c_str());
    }
    
    // Utwórz przycisk do zamykania aplikacji
    HWND buttonHwnd = CreateWindowW(
        L"BUTTON",                 // Nazwa klasy kontrolki
        L"Zamknij",                // Tekst przycisku
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        (m_width - 100) / 2,       // X pozycja (wyśrodkowana)
        (m_height - 30) / 2,       // Y pozycja (wyśrodkowana)
        100,                       // Szerokość
        30,                        // Wysokość
        m_hwnd,                    // Rodzic
        (HMENU)1,                  // ID przycisku
        GetModuleHandle(NULL),     // Instance
        NULL                       // Dodatkowe dane
    );
    
    return true;
}

void SimpleWindow::show() {
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    
    // Usunięcie głównej pętli wiadomości z tej metody
    // Pętla wiadomości jest teraz obsługiwana w głównej funkcji wWinMain
}

void SimpleWindow::close() {
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void SimpleWindow::setHandler(UINT message, std::function<void()> handler) {
    if (message >= WM_APP && message < WM_APP + 100) {
        m_handlers[message - WM_APP] = handler;
    }
}

LRESULT CALLBACK SimpleWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Obsługa wiadomości
    switch (uMsg) {
        case WM_COMMAND:
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
    
    // Sprawdź, czy zarejestrowano handler dla tej wiadomości
    if (uMsg >= WM_APP && uMsg < WM_APP + 100 && 
        s_instance != nullptr && 
        s_instance->m_handlers[uMsg - WM_APP] != nullptr) {
        
        // Wywołaj handler
        s_instance->m_handlers[uMsg - WM_APP]();
        return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}