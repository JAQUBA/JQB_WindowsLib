#include "WindowsApp.h"

namespace WinAppLib {

// Inicjalizacja statycznych członków klasy
std::map<WindowsApp::WindowClassRegistration, ATOM> WindowsApp::s_registeredClasses;

// Konstruktor
WindowsApp::WindowsApp(HINSTANCE hInstance, const std::wstring& appName, const std::wstring& windowTitle)
    : m_hInstance(hInstance)
    , m_hwnd(NULL)
    , m_appName(appName)
    , m_windowTitle(windowTitle)
{
}

// Destruktor
WindowsApp::~WindowsApp() {
    // Nic specjalnego do zrobienia tutaj - okno jest niszczone przez WM_DESTROY
}

// Inicjalizacja aplikacji
bool WindowsApp::initialize() {
    // Zarejestruj klasę okna
    if (!registerWindowClass()) {
        return false;
    }
    
    // Ustawienia okna
    DWORD style = WS_OVERLAPPEDWINDOW;
    DWORD exStyle = 0;
    int x = CW_USEDEFAULT, y = CW_USEDEFAULT;
    int width = 800, height = 600;
    
    // Pozwól klasie pochodnej dostosować parametry
    setupWindowParams(style, exStyle, x, y, width, height);
    
    // Utwórz okno główne
    m_hwnd = CreateWindowEx(
        exStyle,
        m_appName.c_str(),
        m_windowTitle.c_str(),
        style,
        x, y, width, height,
        NULL, NULL, m_hInstance, this
    );
    
    if (!m_hwnd) {
        return false;
    }
    
    return true;
}

// Uruchomienie głównej pętli aplikacji
int WindowsApp::run(int nCmdShow) {
    if (!m_hwnd) {
        return -1;
    }
    
    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

// Rejestracja klasy okna
ATOM WindowsApp::registerWindowClass() {
    WindowClassRegistration reg;
    reg.className = m_appName;
    reg.hInstance = m_hInstance;
    
    // Sprawdź, czy klasa jest już zarejestrowana
    auto it = s_registeredClasses.find(reg);
    if (it != s_registeredClasses.end()) {
        return it->second;
    }
    
    // Przygotuj dane klasy okna
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowsApp::staticWindowProc;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = m_appName.c_str();
    
    // Pozwól klasie pochodnej dostosować klasę okna
    setupWindowClass(wc);
    
    // Zarejestruj klasę
    ATOM atom = RegisterClassEx(&wc);
    if (atom) {
        s_registeredClasses[reg] = atom;
    }
    
    return atom;
}

// Statyczna procedura okna przekierowująca do instancji
LRESULT CALLBACK WindowsApp::staticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    WindowsApp* app = NULL;
    
    if (uMsg == WM_NCCREATE) {
        // Podczas tworzenia okna, zapisz wskaźnik do obiektu
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        app = static_cast<WindowsApp*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)app);
        
        // Wywołaj metodę onCreate
        if (app) {
            app->onCreate(hwnd);
        }
    } else {
        // Pobierz zapisany wskaźnik
        app = (WindowsApp*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }
    
    if (app) {
        return app->windowProc(hwnd, uMsg, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Procedura okna dla konkretnej instancji
LRESULT WindowsApp::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Najpierw sprawdź, czy jest zarejestrowana obsługa dla tego komunikatu
    auto it = m_messageHandlers.find(uMsg);
    if (it != m_messageHandlers.end()) {
        LRESULT result = 0;
        if (it->second(hwnd, uMsg, wParam, lParam, result)) {
            return result; // Obsługa wykonana
        }
    }
    
    // Standardowa obsługa komunikatów
    switch (uMsg) {
        case WM_CREATE:
            // Przygotuj podstawowe kontrolki UI
            createControls();
            return 0;
            
        case WM_DESTROY:
            // Wywołaj metodę onDestroy
            onDestroy();
            PostQuitMessage(0);
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Rejestracja obsługi komunikatu
void WindowsApp::registerMessageHandler(UINT msg, MessageHandlerFunc handler) {
    m_messageHandlers[msg] = handler;
}

// Domyślne implementacje metod wirtualnych

bool WindowsApp::onCreate(HWND hwnd) {
    return true;
}

void WindowsApp::onDestroy() {
    // Domyślna implementacja nic nie robi
}

void WindowsApp::createControls() {
    // Domyślna implementacja nic nie robi
}

void WindowsApp::setupWindowClass(WNDCLASSEX& wc) {
    // Domyślna implementacja nic nie robi
}

void WindowsApp::setupWindowParams(DWORD& style, DWORD& exStyle, int& x, int& y, int& width, int& height) {
    // Domyślna implementacja nic nie robi
}

} // namespace WinAppLib