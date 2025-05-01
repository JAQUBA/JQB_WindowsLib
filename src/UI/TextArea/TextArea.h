#ifndef TEXT_AREA_H
#define TEXT_AREA_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <vector>

class TextArea : public UIComponent {
public:
    TextArea(int x, int y, int width, int height);
    ~TextArea() override;

    void create(HWND parent) override;
    void setText(const char* text);
    void setText(const std::string& text);
    void setText(const wchar_t* text);
    void setText(const std::wstring& text);
    void append(const std::string& text);
    void append(const std::wstring& text);
    void clear();

    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const wchar_t* getText() const { return m_text.c_str(); }
    std::string getTextUTF8() const;
    HWND getHandle() const override { return m_hwnd; }
    int getId() const override { return m_id; }

private:
    // Konwersje między formatami UTF-8 a Unicode
    std::wstring utf8ToWide(const std::string& text) const;
    std::string wideToUtf8(const std::wstring& wstr) const;

    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::wstring m_text;  // Zmienione z std::string na std::wstring
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
};

#endif // TEXT_AREA_H