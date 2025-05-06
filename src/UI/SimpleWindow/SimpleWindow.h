#ifndef SIMPLE_WINDOW_H
#define SIMPLE_WINDOW_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <vector>

// Deklaracje wyprzedzające klas dla zachowania kompatybilności wstecznej
class Button;
class Label;
class Select;
class TextArea;
class ValueDisplay;
class Chart;

class SimpleWindow {
    public:
        SimpleWindow(int width, int height, const char* title, int iconId);
        ~SimpleWindow();

        bool init();
        void close();
        
        // Nowa ujednolicona metoda do dodawania dowolnego komponentu UI
        void add(UIComponent* component);
        
        // Pozostawiam metody przeciążone dla zachowania kompatybilności wstecznej
        // (będą one wewnętrznie korzystały z nowej metody add(UIComponent*))
        void add(Button* button);
        void add(Label* label);
        void add(Select* select);
        void add(TextArea* textArea);
        void add(ValueDisplay* valueDisplay);
        void add(Chart* chart);
        
    private:
        static SimpleWindow* s_instance;
        
        HWND m_hwnd;

        int m_width;
        int m_height;
        std::wstring m_titleW;  // Zmiana z char* na std::wstring
        int m_iconId;

        // Zbiorczy wektor dla wszystkich komponentów UI
        std::vector<UIComponent*> m_components;

        // Pozostawiam stare wektory dla zachowania kompatybilności wstecznej
        std::vector<Button*> m_buttons;
        std::vector<Label*> m_labels;
        std::vector<Select*> m_selects;
        std::vector<TextArea*> m_textAreas;
        std::vector<ValueDisplay*> m_valueDisplays;
        std::vector<Chart*> m_charts;

        // Statyczna funkcja obsługująca wiadomości okna
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // SIMPLE_WINDOW_H