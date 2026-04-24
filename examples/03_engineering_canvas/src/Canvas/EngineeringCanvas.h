#pragma once

#include <UI/CanvasWindow/CanvasWindow.h>

class EngineeringCanvas : public CanvasWindow {
public:
    void setShowOutline(bool value) { m_showOutline = value; }
    void setShowPaths(bool value) { m_showPaths = value; }
    void setPathDensity(int value);

    void zoomToFit(double boardW, double boardH);

protected:
    void onDraw(HDC hdc, const RECT& rc) override;

private:
    bool m_showOutline = true;
    bool m_showPaths = true;
    int m_pathDensity = 16;

    double m_boardW = 120.0;
    double m_boardH = 80.0;
};
