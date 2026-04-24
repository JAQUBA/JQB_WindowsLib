#include "DocumentCanvas.h"

#include <windows.h>

#include <sstream>

void DocumentCanvas::setSource(const std::string& source) {
    m_lineLengths.clear();

    std::stringstream ss(source);
    std::string line;
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            m_lineLengths.push_back(line.size());
        }
    }

    if (m_lineLengths.empty()) {
        m_lineLengths.push_back(0);
    }
}

void DocumentCanvas::onDraw(HDC hdc, const RECT&) {
    HPEN framePen = CreatePen(PS_SOLID, 1, RGB(220, 220, 80));
    HPEN oldPen = (HPEN)SelectObject(hdc, framePen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(
        hdc,
        toScreenX(0.0),
        toScreenY(m_worldH),
        toScreenX(m_worldW),
        toScreenY(0.0));

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(framePen);

    HPEN linePen = CreatePen(PS_SOLID, 2, RGB(80, 200, 240));
    oldPen = (HPEN)SelectObject(hdc, linePen);

    const size_t maxLines = 12;
    size_t count = m_lineLengths.size();
    if (count > maxLines) count = maxLines;

    for (size_t i = 0; i < count; ++i) {
        double y = m_worldH - 10.0 - (double)i * 8.5;

        size_t len = m_lineLengths[i];
        if (len > 70) len = 70;

        double width = 12.0 + (double)len * 2.1;
        if (width > m_worldW - 16.0) width = m_worldW - 16.0;

        MoveToEx(hdc, toScreenX(8.0), toScreenY(y), NULL);
        LineTo(hdc, toScreenX(8.0 + width), toScreenY(y));
    }

    SelectObject(hdc, oldPen);
    DeleteObject(linePen);
}
