#ifndef LABEL_H
#define LABEL_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>

class Label : public UIComponent {
public:
    Label(int x, int y, int width, int height, const wchar_t* text);
    ~Label() override;

    void create(HWND parent) override;
    void setText(const wchar_t* text);

    // Stylowanie czcionki
    void setFont(const wchar_t* fontName, int size, bool bold = false, bool italic = false);

    // Stylowanie kolorów (wymaga obsługi WM_CTLCOLORSTATIC w oknie nadrzędnym)
    void setTextColor(COLORREF color);
    void setBackColor(COLORREF color);
    bool hasCustomColors() const;
    COLORREF getTextColor() const { return m_textColor; }
    COLORREF getBackColor() const { return m_backColor; }
    HBRUSH   getBackBrush() const { return m_hBackBrush; }

    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const wchar_t* getText() const { return m_text.c_str(); }
    HWND getHandle() const override { return m_hwnd; }
    int getId() const override { return m_id; }

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::wstring m_text;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;

    HFONT    m_hFont      = NULL;
    COLORREF m_textColor  = CLR_INVALID;
    COLORREF m_backColor  = CLR_INVALID;
    HBRUSH   m_hBackBrush = NULL;
};

#endif // LABEL_H