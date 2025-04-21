#ifndef TEXT_AREA_H
#define TEXT_AREA_H

#include "Core.h"
#include <string>
#include <vector>

class TextArea {
public:
    TextArea(int x, int y, int width, int height);
    ~TextArea();

    void create(HWND parent);
    void setText(const char* text);
    void append(const std::string& text);
    void clear();

    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const char* getText() const { return m_text.c_str(); }
    HWND getHandle() const { return m_hwnd; }
    int getId() const { return m_id; }

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::string m_text;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
};

#endif // TEXT_AREA_H