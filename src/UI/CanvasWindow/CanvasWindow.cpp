// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// CanvasWindow.cpp — Zoomable/pannable GDI canvas implementation
// ============================================================================
#include "CanvasWindow.h"
#include <windowsx.h>

const wchar_t* CanvasWindow::CLASS_NAME = L"JQB_CanvasWindow";
bool CanvasWindow::s_classRegistered = false;

CanvasWindow::CanvasWindow() {}

CanvasWindow::~CanvasWindow() {
    destroyResources();
    if (m_hwnd) DestroyWindow(m_hwnd);
}

// ============================================================================
// Window creation
// ============================================================================
void CanvasWindow::create(HWND parent, int x, int y, int w, int h) {
    m_parent = parent;

    if (!s_classRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursorW(nullptr, (LPCWSTR)IDC_ARROW);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
        RegisterClassW(&wc);
        s_classRegistered = true;
    }

    m_hwnd = CreateWindowExW(
        0, CLASS_NAME, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
        x, y, w, h,
        parent, nullptr, GetModuleHandleW(nullptr), this);

    createResources();
}

// ============================================================================
// GDI Resources
// ============================================================================
void CanvasWindow::createResources() {
    m_penGrid = CreatePen(PS_SOLID, 1, m_gridColor);
    m_brushBg = CreateSolidBrush(m_bgColor);
}

void CanvasWindow::destroyResources() {
    if (m_penGrid) { DeleteObject(m_penGrid); m_penGrid = nullptr; }
    if (m_brushBg) { DeleteObject(m_brushBg); m_brushBg = nullptr; }
}

// ============================================================================
// Public API
// ============================================================================
void CanvasWindow::redraw() {
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, FALSE);
}

void CanvasWindow::resetView() {
    m_zoom = m_defaultZoom;
    m_panX = m_defaultPanX;
    m_panY = m_defaultPanY;
    redraw();
}

void CanvasWindow::setGridVisible(bool visible) {
    m_gridVisible = visible;
    redraw();
}

void CanvasWindow::setGridSpacing(double spacingMm) {
    m_gridSpacing = spacingMm;
    redraw();
}

void CanvasWindow::setGridExtent(double widthMm, double heightMm) {
    m_gridWidth = widthMm;
    m_gridHeight = heightMm;
    redraw();
}

void CanvasWindow::setGridColor(COLORREF color) {
    m_gridColor = color;
    if (m_penGrid) DeleteObject(m_penGrid);
    m_penGrid = CreatePen(PS_SOLID, 1, m_gridColor);
    redraw();
}

void CanvasWindow::setBackgroundColor(COLORREF color) {
    m_bgColor = color;
    if (m_brushBg) DeleteObject(m_brushBg);
    m_brushBg = CreateSolidBrush(m_bgColor);
    redraw();
}

void CanvasWindow::setDefaultZoom(double zoom) {
    m_defaultZoom = zoom;
    m_zoom = zoom;
}

void CanvasWindow::setDefaultPan(double panX, double panY) {
    m_defaultPanX = panX;
    m_defaultPanY = panY;
    m_panX = panX;
    m_panY = panY;
}

// ============================================================================
// Coordinate transforms
// ============================================================================
int CanvasWindow::toScreenX(double worldX) const {
    return static_cast<int>(worldX * m_zoom * 10.0 + m_panX);
}

int CanvasWindow::toScreenY(double worldY) const {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    return rc.bottom - static_cast<int>(worldY * m_zoom * 10.0 + m_panY);
}

// ============================================================================
// Drawing
// ============================================================================
void CanvasWindow::onPaint(HDC hdc, const RECT& rc) {
    FillRect(hdc, &rc, m_brushBg);
    if (m_gridVisible) drawGrid(hdc);
    onDraw(hdc, rc);
}

void CanvasWindow::drawGrid(HDC hdc) {
    SelectObject(hdc, m_penGrid);

    int maxY = static_cast<int>(m_gridHeight);
    int maxX = static_cast<int>(m_gridWidth);
    int step = static_cast<int>(m_gridSpacing);
    if (step < 1) step = 1;

    for (int y = 0; y <= maxY; y += step) {
        int sy = toScreenY(static_cast<double>(y));
        MoveToEx(hdc, toScreenX(0), sy, nullptr);
        LineTo(hdc, toScreenX(static_cast<double>(maxX)), sy);
    }

    for (int x = 0; x <= maxX; x += step) {
        int sx = toScreenX(static_cast<double>(x));
        MoveToEx(hdc, sx, toScreenY(0), nullptr);
        LineTo(hdc, sx, toScreenY(static_cast<double>(maxY)));
    }
}

// ============================================================================
// Window Procedure
// ============================================================================
LRESULT CALLBACK CanvasWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    CanvasWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<CanvasWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<CanvasWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

            self->onPaint(memDC, rc);

            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBmp);
            DeleteObject(memBmp);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(hwnd, &pt);

            double oldZoom = self->m_zoom;
            double zoomDelta = (delta > 0) ? 0.02 : -0.02;
            self->m_zoom += zoomDelta;
            if (self->m_zoom < 0.02) self->m_zoom = 0.02;
            if (self->m_zoom > 5.0) self->m_zoom = 5.0;

            double ratio = self->m_zoom / oldZoom;
            self->m_panX = pt.x - ratio * (pt.x - self->m_panX);
            self->m_panY = pt.y - ratio * (pt.y - self->m_panY);

            self->redraw();
            return 0;
        }

        case WM_LBUTTONDOWN: {
            self->m_dragging = true;
            self->m_dragStart.x = GET_X_LPARAM(lParam);
            self->m_dragStart.y = GET_Y_LPARAM(lParam);
            self->m_dragPanStartX = self->m_panX;
            self->m_dragPanStartY = self->m_panY;
            SetCapture(hwnd);
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (self->m_dragging) {
                int dx = GET_X_LPARAM(lParam) - self->m_dragStart.x;
                int dy = GET_Y_LPARAM(lParam) - self->m_dragStart.y;
                self->m_panX = self->m_dragPanStartX + dx;
                self->m_panY = self->m_dragPanStartY - dy;
                self->redraw();
            }
            return 0;
        }

        case WM_LBUTTONUP: {
            self->m_dragging = false;
            ReleaseCapture();
            return 0;
        }

        case WM_LBUTTONDBLCLK: {
            self->resetView();
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
