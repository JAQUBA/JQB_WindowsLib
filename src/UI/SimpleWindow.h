#ifndef SIMPLE_WINDOW_H
#define SIMPLE_WINDOW_H

#include <windows.h>
#include <functional>
#include <string>

class SimpleWindow {
public:
    // Konstruktor okna
    SimpleWindow(int width, int height, const std::wstring& title, int iconId);
    ~SimpleWindow();

    // Inicjalizacja okna
    bool init();
    
    // Wyświetlenie okna
    void show();
    
    // Zamknięcie okna
    void close();
    
    // Ustawienie handlera dla eventów
    void setHandler(UINT message, std::function<void()> handler);
    
private:
    // Statyczna funkcja obsługująca wiadomości okna
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // Dane okna
    HWND m_hwnd;
    int m_width;
    int m_height;
    std::wstring m_title;
    int m_iconId;
    
    // Mapa handlerów wiadomości
    std::function<void()> m_handlers[WM_APP + 100];
    
    // Wskaźnik na instancję okna używany w procedurze obsługi wiadomości
    static SimpleWindow* s_instance;
};

#endif // SIMPLE_WINDOW_H