#include "EngineeringCanvas.h"

#include <windows.h>

void EngineeringCanvas::setPathDensity(int value) {
    if (value < 4) value = 4;
    if (value > 60) value = 60;
    m_pathDensity = value;
}

void EngineeringCanvas::zoomToFit(double boardW, double boardH) {
    if (!m_hwnd || boardW <= 0.0 || boardH <= 0.0) return;

    m_boardW = boardW;
    m_boardH = boardH;

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    int cw = rc.right - rc.left;
    int ch = rc.bottom - rc.top;
    if (cw <= 0 || ch <= 0) return;

    double zx = (cw * 0.85) / (m_boardW * 10.0);
    double zy = (ch * 0.85) / (m_boardH * 10.0);
    double z = (zx < zy) ? zx : zy;

    double cx = m_boardW * 0.5;
    double cy = m_boardH * 0.5;

    double panX = (cw * 0.5) - (cx * z * 10.0);
    double panY = (ch * 0.5) - (cy * z * 10.0);

    setDefaultZoom(z);
    setDefaultPan(panX, panY);
    resetView();
}

void EngineeringCanvas::onDraw(HDC hdc, const RECT&) {
    const COLORREF clrOutline = RGB(240, 210, 90);
    const COLORREF clrPaths = RGB(80, 200, 240);

    if (m_showOutline) {
        HPEN pen = CreatePen(PS_SOLID, 2, clrOutline);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(
            hdc,
            toScreenX(0.0),
            toScreenY(m_boardH),
            toScreenX(m_boardW),
            toScreenY(0.0));

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    if (m_showPaths) {
        HPEN pen = CreatePen(PS_SOLID, 1, clrPaths);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        for (int i = 0; i < m_pathDensity; ++i) {
            double y = 4.0 + (m_boardH - 8.0) * (double)i / (double)(m_pathDensity - 1);
            double bias = (i % 2 == 0) ? 2.2 : -2.2;

            MoveToEx(hdc, toScreenX(4.0), toScreenY(y), NULL);
            LineTo(hdc, toScreenX(m_boardW - 4.0), toScreenY(y + bias));
        }

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }
}
