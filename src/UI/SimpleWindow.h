#ifndef SIMPLE_WINDOW_H
#define SIMPLE_WINDOW_H

#include "Core.h"
#include <string>

class SimpleWindow {
    public:

        SimpleWindow(int width, int height, const std::wstring& title, int iconId);
        ~SimpleWindow();

        bool init();

        void close();
        
    private:
        static SimpleWindow* s_instance;
        
        HWND m_hwnd;

        int m_width;
        int m_height;
        std::wstring m_title;
        int m_iconId;

        // Statyczna funkcja obsługująca wiadomości okna
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // SIMPLE_WINDOW_H