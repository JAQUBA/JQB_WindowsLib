#ifndef SELECT_H
#define SELECT_H

#include "Core.h"
#include <string>
#include <functional>
#include <vector>

class Select {
public:
    Select(int x, int y, int width, int height, const char* text, std::function<void(Select*)> onChange);
    ~Select();

    void create(HWND parent);
    void handleSelection();
    void addItem(const char* item);
    void setText(const char* text);
    void clear();
    void link(const std::vector<std::string>* items);
    void updateItems();
    
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
    std::vector<std::string> m_items;
    const std::vector<std::string>* m_linkedItems;
    int m_selectedIndex;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
    std::function<void(Select*)> m_onChange;
};

#endif // SELECT_H