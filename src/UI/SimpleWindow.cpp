#include "SimpleWindow.h"

// Inicjalizacja statycznej zmiennej
SimpleWindow* SimpleWindow::s_instance = nullptr;

SimpleWindow::SimpleWindow(int width, int height, const std::wstring& title, int iconId)
    : m_width(width), m_height(height), m_title(title), m_iconId(iconId), m_hwnd(NULL) {
    s_instance = this;


}

SimpleWindow::~SimpleWindow() {
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
        m_title.c_str(),
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
    
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}