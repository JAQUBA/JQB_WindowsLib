#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "Core.h"
#include "../UIComponent.h"
#include <functional>

class ProgressBar : public UIComponent {
public:
    ProgressBar(int x, int y, int width, int height, int minimum = 0, int maximum = 100);
    ~ProgressBar();
    
    void create(HWND parent) override;
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }
    
    // Metody specyficzne dla ProgressBar
    void setProgress(int value);
    int getProgress() const;
    void setRange(int minimum, int maximum);
    void getRange(int& minimum, int& maximum) const;
    void setColor(COLORREF color);
    void setBackColor(COLORREF color);
    void setMarquee(bool enable, int interval = 30);
    void step(int increment = 1);

private:
    int m_x, m_y, m_width, m_height;
    int m_minimum, m_maximum, m_value;
    COLORREF m_color, m_backColor;
    bool m_isMarquee;
    int m_marqueeInterval;
    HWND m_hwnd;
    int m_id;
    
    static int s_nextId;
};

#endif // PROGRESSBAR_H