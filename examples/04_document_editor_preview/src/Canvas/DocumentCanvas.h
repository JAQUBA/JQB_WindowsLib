#pragma once

#include <UI/CanvasWindow/CanvasWindow.h>

#include <string>
#include <vector>

class DocumentCanvas : public CanvasWindow {
public:
    void setSource(const std::string& source);

protected:
    void onDraw(HDC hdc, const RECT& rc) override;

private:
    std::vector<size_t> m_lineLengths;
    double m_worldW = 180.0;
    double m_worldH = 120.0;
};
