// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

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
        bool isNewSegment = false;
    };

    Chart(int x, int y, int width, int height, const char* title = "Wykres pomiarów");
    ~Chart() override;

    void create(HWND parent) override;
    void addDataPoint(double value, const std::wstring& unit);
    void addDataPoints(const double* values, int count, double totalDurationMs);
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
    void setTimeWindow(double seconds) { m_timeWindowSec = seconds; }
    void setColors(COLORREF gridColor, COLORREF axisColor, COLORREF dataColor);
    void setAutoScale(bool autoScale) { m_autoScale = autoScale; }
    void setYRange(double minY, double maxY) {
        m_manualMinY = minY;
        m_manualMaxY = maxY;
        m_autoScale = false;
    }
    
    // Ustawienie limitu odświeżania
    void setRefreshRate(int millisecondsInterval) { m_refreshInterval = millisecondsInterval; }

    // Trigger mode — oscilloscope-style rising zero-crossing sync
    void setTriggerEnabled(bool enabled) { m_triggerEnabled = enabled; }

    // Line width for data rendering (default: 2)
    void setLineWidth(int width) { m_lineWidth = width; }

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
    double m_timeWindowSec = 30.0; // Domyślnie pokazuje 30 sekund
    
    COLORREF m_gridColor = RGB(80, 80, 80);
    COLORREF m_axisColor = RGB(200, 200, 200);
    COLORREF m_dataColor = RGB(0, 255, 0);
    
    bool m_autoScale = true;
    double m_manualMinY = 0.0;
    double m_manualMaxY = 10.0;
    
    // Zmienne do ograniczania częstotliwości odświeżania
    int m_refreshInterval = 100; // Domyślnie 100ms (10 FPS)
    std::chrono::steady_clock::time_point m_lastRefreshTime;
    bool m_dataChanged = false; // Flaga wskazująca, że dane się zmieniły od ostatniego odświeżenia
    
    // Pomocnicze funkcje do rysowania
    void drawGrid(HDC hdc, const RECT& rect);
    void drawAxes(HDC hdc, const RECT& rect);
    void drawData(HDC hdc, const RECT& rect);
    
    // Funkcja do usuwania starych punktów danych
    void cleanOldDataPoints();
    
    // Trigger mode (oscilloscope-style zero-crossing sync)
    bool m_triggerEnabled = false;
    int m_lineWidth = 2;

    // Tracking batch continuity
    std::chrono::steady_clock::time_point m_lastBatchEnd;
    bool m_hasBatchHistory = false;
    
    // Funkcje pomocnicze do skalowania danych
    double getMinValue() const;
    double getMaxValue() const;
};

#endif // CHART_H