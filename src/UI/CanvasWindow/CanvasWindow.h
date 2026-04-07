// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// CanvasWindow — Zoomable/pannable GDI canvas (child window)
// ============================================================================
// Reusable base class for 2D vector rendering with:
// - Mouse wheel zoom (towards cursor)
// - Left-button drag to pan
// - Double-click to reset view
// - Optional grid overlay
// - Double-buffered GDI painting (no flicker)
// - World-to-screen coordinate transforms (mm → px)
//
// Subclass overrides onDraw(HDC, RECT) to render custom content.
// Use toScreenX() / toScreenY() for coordinate conversion.
// ============================================================================
#ifndef JQB_CANVAS_WINDOW_H
#define JQB_CANVAS_WINDOW_H

#include "Core.h"
#include <string>

class CanvasWindow {
public:
    CanvasWindow();
    virtual ~CanvasWindow();

    void create(HWND parent, int x, int y, int w, int h);
    void redraw();
    void resetView();
    HWND getHandle() const { return m_hwnd; }

    // Grid
    void setGridVisible(bool visible);
    bool isGridVisible() const { return m_gridVisible; }
    void setGridSpacing(double spacingMm);
    void setGridExtent(double widthMm, double heightMm);
    void setGridColor(COLORREF color);

    // Background
    void setBackgroundColor(COLORREF color);

    // Initial view
    void setDefaultZoom(double zoom);
    void setDefaultPan(double panX, double panY);

protected:
    // Override in subclass to draw custom content.
    // HDC has double-buffered context. Use toScreenX/Y for transforms.
    virtual void onDraw(HDC hdc, const RECT& clientRect) {}

    // World-to-screen coordinate transforms
    int toScreenX(double worldX) const;
    int toScreenY(double worldY) const;

    // Access to view state
    double getZoom() const { return m_zoom; }
    double getPanX() const { return m_panX; }
    double getPanY() const { return m_panY; }

    HWND m_hwnd = nullptr;

private:
    HWND m_parent = nullptr;
    bool m_gridVisible = true;

    // View transform
    double m_zoom = 0.15;
    double m_panX = 10.0;
    double m_panY = 10.0;
    double m_defaultZoom = 0.15;
    double m_defaultPanX = 10.0;
    double m_defaultPanY = 10.0;

    // Mouse drag state
    bool m_dragging = false;
    POINT m_dragStart = {};
    double m_dragPanStartX = 0.0;
    double m_dragPanStartY = 0.0;

    // Grid settings
    double m_gridSpacing = 10.0;   // mm
    double m_gridWidth = 210.0;    // mm
    double m_gridHeight = 300.0;   // mm

    // GDI resources
    HPEN m_penGrid = nullptr;
    HBRUSH m_brushBg = nullptr;
    COLORREF m_gridColor = RGB(60, 60, 80);
    COLORREF m_bgColor = RGB(22, 22, 28);

    void createResources();
    void destroyResources();

    void onPaint(HDC hdc, const RECT& rc);
    void drawGrid(HDC hdc);

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static const wchar_t* CLASS_NAME;
    static bool s_classRegistered;
};

#endif // JQB_CANVAS_WINDOW_H
