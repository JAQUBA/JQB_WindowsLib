#ifndef SIMPLE_WINDOW_H
#define SIMPLE_WINDOW_H

#include "Core.h"
#include <string>
#include <vector>

class Button;
class Label;
class Select;
class TextArea;

class SimpleWindow {
    public:

        SimpleWindow(int width, int height, const char* title, int iconId);
        ~SimpleWindow();

        bool init();
        void close();
        
        // Dodawanie kontrolek do okna
        void add(Button* button);
        void add(Label* label);
        void add(Select* select);
        void add(TextArea* textArea);
        
    private:
        static SimpleWindow* s_instance;
        
        HWND m_hwnd;

        int m_width;
        int m_height;
        const char *m_title;
        int m_iconId;

        // Lista kontrolek
        std::vector<Button*> m_buttons;
        std::vector<Label*> m_labels;
        std::vector<Select*> m_selects;
        std::vector<TextArea*> m_textAreas;

        // Statyczna funkcja obsługująca wiadomości okna
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // SIMPLE_WINDOW_H