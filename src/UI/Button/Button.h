#ifndef BUTTON_H
#define BUTTON_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <functional>

// Deklaracje funkcji pomocniczych do obsługi naciśnięć przycisków
void startButtonPress(int buttonId);
void endButtonPress(int buttonId);
void checkForLongPresses();

// Deklaracja wyprzedzająca dla funkcji przyjaciela
LRESULT CALLBACK CustomButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class Button : public UIComponent {
public:
    Button(int x, int y, int width, int height, const char* text, 
           std::function<void(Button*)> onClick,
           std::function<void(Button*)> onLongClick = nullptr);
    ~Button() override;

    void create(HWND parent) override;
    void handleClick() override;
    void handleLongClick();

    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const char* getText() const { return m_text.c_str(); }
    HWND getHandle() const override { return m_hwnd; }
    int getId() const override { return m_id; }

    // Deklaracje funkcji zaprzyjaźnionych
    friend void startButtonPress(int buttonId);
    friend void endButtonPress(int buttonId);
    friend void checkForLongPresses();
    friend LRESULT CALLBACK ButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend LRESULT CALLBACK CustomButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::string m_text;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
    std::function<void(Button*)> m_onClick;
    std::function<void(Button*)> m_onLongClick;
};

#endif // BUTTON_H