// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef SIMPLE_WINDOW_H
#define SIMPLE_WINDOW_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <vector>
#include <functional>

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
        
        // Uchwyt okna
        HWND getHandle() const { return m_hwnd; }
        
        // Pasek menu
        void setMenu(HMENU menu);
        void onMenuCommand(std::function<void(int)> callback) { m_onMenuCommand = callback; }
        
        // Kolor tła okna
        void setBackgroundColor(COLORREF color);
        
        // Domyślny kolor tekstu (dla kontrolek bez własnego koloru)
        void setTextColor(COLORREF color) { m_textColor = color; }
        
        // Callback wywoływany przed zamknięciem okna
        void onClose(std::function<void()> callback) { m_onCloseCallback = callback; }
        
        // Callback do obsługi rysowania własnych kontrolek (owner-draw)
        void onDrawItem(std::function<bool(DRAWITEMSTRUCT*)> callback) { m_onDrawItem = callback; }
        
        // Nowa ujednolicona metoda do dodawania dowolnego komponentu UI
        // parent — optional parent HWND (e.g. tab page); defaults to main window
        void add(UIComponent* component, HWND parent = NULL);
        
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
        
        // Callbacki
        std::function<void()> m_onCloseCallback;
        std::function<void(int)> m_onMenuCommand;
        std::function<bool(DRAWITEMSTRUCT*)> m_onDrawItem;
        
        // Kolor tła
        COLORREF m_backColor = CLR_INVALID;
        COLORREF m_textColor = CLR_INVALID;
        HBRUSH m_hBackBrush = NULL;
        HBRUSH m_hCtrlBrush = NULL;

        // Statyczna funkcja obsługująca wiadomości okna
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif // SIMPLE_WINDOW_H