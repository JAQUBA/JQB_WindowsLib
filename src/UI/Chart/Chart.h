#ifndef CHART_H
#define CHART_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <functional>
#include <map>

class Chart : public UIComponent {
public:
    struct DataPoint {
        double value;
        std::chrono::steady_clock::time_point timestamp;
        std::wstring unit;
    };

    Chart(int x, int y, int width, int height, const char* title = "Wykres pomiarów");
    ~Chart() override;

    void create(HWND parent) override;
    void addDataPoint(double value, const std::wstring& unit);
    void render(HDC hdc);
    void clear();
    
    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    const char* getTitle() const { return m_title.c_str(); }
    HWND getHandle() const override { return m_hwnd; }
    int getId() const override { return m_id; }

    // Ustawienia wykresu
    void setTimeWindow(int seconds) { m_timeWindowSeconds = seconds; }
    void setColors(COLORREF gridColor, COLORREF axisColor, COLORREF dataColor);
    void setAutoScale(bool autoScale) { m_autoScale = autoScale; }
    void setYRange(double minY, double maxY) {
        m_manualMinY = minY;
        m_manualMaxY = maxY;
        m_autoScale = false;
    }

private:
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    std::string m_title;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;

    std::deque<DataPoint> m_dataPoints;
    int m_timeWindowSeconds = 30; // Domyślnie pokazuje 30 sekund
    
    COLORREF m_gridColor = RGB(80, 80, 80);
    COLORREF m_axisColor = RGB(200, 200, 200);
    COLORREF m_dataColor = RGB(0, 255, 0);
    
    bool m_autoScale = true;
    double m_manualMinY = 0.0;
    double m_manualMaxY = 10.0;
    
    // Pomocnicze funkcje do rysowania
    void drawGrid(HDC hdc, const RECT& rect);
    void drawAxes(HDC hdc, const RECT& rect);
    void drawData(HDC hdc, const RECT& rect);
    
    // Funkcja do usuwania starych punktów danych
    void cleanOldDataPoints();
    
    // Funkcje pomocnicze do skalowania danych
    double getMinValue() const;
    double getMaxValue() const;
};

#endif // CHART_H