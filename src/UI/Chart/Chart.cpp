#include "Chart.h"
#include "../../Util/StringUtils.h"
#include <algorithm>
#include <numeric>
#include <cmath> // Dodaję nagłówek cmath dla funkcji fabs

// Inicjalizacja statycznej zmiennej
int Chart::s_nextId = 5000;

// Procedura obsługi okna wykresu
LRESULT CALLBACK ChartProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Chart* chart = reinterpret_cast<Chart*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Rysowanie wykresu
            if (chart) {
                chart->render(hdc);
            }
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

Chart::Chart(int x, int y, int width, int height, const char* title)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_title(title), m_hwnd(NULL) {
    m_id = s_nextId++;
}

Chart::~Chart() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void Chart::create(HWND parent) {
    // Rejestracja klasy okna dla wykresu
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = ChartProc;
        wc.hInstance = _core.hInstance;
        wc.lpszClassName = L"ChartClass";
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        
        RegisterClassW(&wc);
        registered = true;
    }
    
    // Konwersja tytułu z UTF-8 na UTF-16
    std::wstring wideTitle = StringUtils::utf8ToWide(m_title);
    
    // Utworzenie okna wykresu
    m_hwnd = CreateWindowExW(
        0,
        L"ChartClass",
        wideTitle.c_str(),
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        NULL
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć wykresu!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Ustawienie wskaźnika do instancji klasy
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
}

void Chart::addDataPoint(double value, const std::wstring& unit) {
    DataPoint point;
    point.value = value;
    point.timestamp = std::chrono::steady_clock::now();
    point.unit = unit;
    
    m_dataPoints.push_back(point);
    
    // Usunięcie starych punktów danych
    cleanOldDataPoints();
    
    // Oznacz, że dane się zmieniły
    m_dataChanged = true;
    
    // Odświeżenie wykresu z ograniczeniem częstotliwości
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRefreshTime).count();
    
    if (elapsed >= m_refreshInterval) {
        m_lastRefreshTime = now;
        m_dataChanged = false;
        
        if (m_hwnd) {
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }
}

void Chart::cleanOldDataPoints() {
    if (m_dataPoints.empty()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto cutoff = now - std::chrono::seconds(m_timeWindowSeconds);
    
    // Usuń punkty starsze niż okno czasowe
    while (!m_dataPoints.empty() && m_dataPoints.front().timestamp < cutoff) {
        m_dataPoints.pop_front();
    }
}

void Chart::clear() {
    m_dataPoints.clear();
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void Chart::setColors(COLORREF gridColor, COLORREF axisColor, COLORREF dataColor) {
    m_gridColor = gridColor;
    m_axisColor = axisColor;
    m_dataColor = dataColor;
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

double Chart::getMinValue() const {
    if (m_dataPoints.empty()) {
        return m_manualMinY;
    }
    
    if (!m_autoScale) {
        return m_manualMinY;
    }
    
    double minVal = m_dataPoints.front().value;
    for (const auto& point : m_dataPoints) {
        minVal = std::min(minVal, point.value);
    }
    
    // Dodaj trochę marginesu
    return minVal - std::abs(minVal * 0.1);
}

double Chart::getMaxValue() const {
    if (m_dataPoints.empty()) {
        return m_manualMaxY;
    }
    
    if (!m_autoScale) {
        return m_manualMaxY;
    }
    
    double maxVal = m_dataPoints.front().value;
    for (const auto& point : m_dataPoints) {
        maxVal = std::max(maxVal, point.value);
    }
    
    // Dodaj trochę marginesu
    return maxVal + std::abs(maxVal * 0.1);
}

void Chart::render(HDC hdc) {
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    // Wypełnij tło
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(hdc, &clientRect, bgBrush);
    DeleteObject(bgBrush);
    
    // Rysuj komponenty wykresu
    drawGrid(hdc, clientRect);
    drawAxes(hdc, clientRect);
    drawData(hdc, clientRect);
    
    // Rysuj tytuł
    std::wstring wideTitle = StringUtils::utf8ToWide(m_title);
    HFONT font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");
    
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    
    RECT titleRect = clientRect;
    titleRect.bottom = 20;
    DrawTextW(hdc, wideTitle.c_str(), -1, &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    
    SelectObject(hdc, oldFont);
    DeleteObject(font);
}

void Chart::drawGrid(HDC hdc, const RECT& rect) {
    // Utwórz pióro dla siatki
    HPEN gridPen = CreatePen(PS_DOT, 1, m_gridColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, gridPen);
    
    // Rysuj poziome linie siatki
    const int horizontalLines = 4;
    int stepY = (rect.bottom - rect.top - 40) / horizontalLines;
    
    for (int i = 1; i <= horizontalLines; i++) {
        int y = rect.bottom - 20 - i * stepY;
        MoveToEx(hdc, rect.left + 50, y, NULL);
        LineTo(hdc, rect.right - 10, y);
    }
    
    // Rysuj pionowe linie siatki
    const int verticalLines = 6;
    int stepX = (rect.right - rect.left - 60) / verticalLines;
    
    for (int i = 1; i <= verticalLines; i++) {
        int x = rect.left + 50 + i * stepX;
        MoveToEx(hdc, x, rect.top + 20, NULL);
        LineTo(hdc, x, rect.bottom - 20);
    }
    
    // Przywróć oryginalne pióro i usuń utworzone
    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);
}

void Chart::drawAxes(HDC hdc, const RECT& rect) {
    // Utwórz pióro dla osi
    HPEN axisPen = CreatePen(PS_SOLID, 2, m_axisColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, axisPen);
    
    // Oś Y
    MoveToEx(hdc, rect.left + 50, rect.top + 20, NULL);
    LineTo(hdc, rect.left + 50, rect.bottom - 20);
    
    // Oś X
    MoveToEx(hdc, rect.left + 50, rect.bottom - 20, NULL);
    LineTo(hdc, rect.right - 10, rect.bottom - 20);
    
    // Etykiety osi Y
    HFONT font = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");
    
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, m_axisColor);
    SetBkMode(hdc, TRANSPARENT);
    
    // Pozyskaj zakres wartości Y
    double minY = getMinValue();
    double maxY = getMaxValue();
    
    const int horizontalLines = 4;
    int stepY = (rect.bottom - rect.top - 40) / horizontalLines;
    
    // Etykiety osi Y
    for (int i = 0; i <= horizontalLines; i++) {
        int y = rect.bottom - 20 - i * stepY;
        double value = minY + (maxY - minY) * i / horizontalLines;
        
        // Formatowanie etykiety
        wchar_t label[32];
        swprintf(label, 32, L"%.2f", value);
        
        RECT labelRect = {rect.left, y - 8, rect.left + 48, y + 8};
        DrawTextW(hdc, label, -1, &labelRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
    
    // Etykiety osi X (czas w sekundach)
    const int verticalLines = 6;
    int stepX = (rect.right - rect.left - 60) / verticalLines;
    
    for (int i = 0; i <= verticalLines; i++) {
        int x = rect.left + 50 + i * stepX;
        int timeValue = m_timeWindowSeconds - m_timeWindowSeconds * i / verticalLines;
        
        // Formatowanie etykiety czasu
        wchar_t label[32];
        swprintf(label, 32, L"-%ds", timeValue);
        
        RECT labelRect = {x - 20, rect.bottom - 20, x + 20, rect.bottom};
        DrawTextW(hdc, label, -1, &labelRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
    }
    
    // Jednostka miary (z ostatniego punktu danych)
    if (!m_dataPoints.empty()) {
        const std::wstring& unit = m_dataPoints.back().unit;
        RECT unitRect = {rect.left, rect.top, rect.left + 50, rect.top + 20};
        DrawTextW(hdc, unit.c_str(), -1, &unitRect, DT_LEFT | DT_TOP);
    }
    
    // Przywróć oryginalne obiekty i usuń utworzone
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    
    SelectObject(hdc, oldPen);
    DeleteObject(axisPen);
}

void Chart::drawData(HDC hdc, const RECT& rect) {
    if (m_dataPoints.empty()) {
        return;
    }
    
    // Utwórz pióro dla danych
    HPEN dataPen = CreatePen(PS_SOLID, 2, m_dataColor);
    HPEN oldPen = (HPEN)SelectObject(hdc, dataPen);
    
    // Zakres wartości Y
    double minY = getMinValue();
    double maxY = getMaxValue();
    
    // Jeśli min i max są takie same (płaska linia), dodaj margines
    if (std::fabs(maxY - minY) < 0.001) {
        minY = minY * 0.9;
        maxY = maxY * 1.1;
        
        // Specjalny przypadek dla zera
        if (std::fabs(minY) < 0.001 && std::fabs(maxY) < 0.001) {
            minY = -1.0;
            maxY = 1.0;
        }
    }
    
    // Szerokość i wysokość obszaru wykresu
    int chartWidth = rect.right - rect.left - 60;
    int chartHeight = rect.bottom - rect.top - 40;
    
    // Obecny czas
    auto now = std::chrono::steady_clock::now();
    
    // Rysowanie linii łączącej punkty danych
    bool first = true;
    for (const auto& point : m_dataPoints) {
        // Oblicz pozycję X na podstawie czasu
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(now - point.timestamp).count();
        double xRatio = 1.0 - (double)timeDiff / (m_timeWindowSeconds * 1000.0);
        
        if (xRatio < 0) continue;  // Punkt poza zakresem czasu
        
        // Oblicz pozycję Y na podstawie wartości
        double yRatio = (point.value - minY) / (maxY - minY);
        if (yRatio < 0) yRatio = 0;
        if (yRatio > 1) yRatio = 1;
        
        int x = rect.left + 50 + static_cast<int>(xRatio * chartWidth);
        int y = rect.bottom - 20 - static_cast<int>(yRatio * chartHeight);
        
        if (first) {
            MoveToEx(hdc, x, y, NULL);
            first = false;
        } else {
            LineTo(hdc, x, y);
        }
    }
    
    // Przywróć oryginalne pióro i usuń utworzone
    SelectObject(hdc, oldPen);
    DeleteObject(dataPen);
    
    // Narysuj punkty danych
    for (const auto& point : m_dataPoints) {
        // Oblicz pozycję X na podstawie czasu
        auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(now - point.timestamp).count();
        double xRatio = 1.0 - (double)timeDiff / (m_timeWindowSeconds * 1000.0);
        
        if (xRatio < 0) continue;  // Punkt poza zakresem czasu
        
        // Oblicz pozycję Y na podstawie wartości
        double yRatio = (point.value - minY) / (maxY - minY);
        if (yRatio < 0) yRatio = 0;
        if (yRatio > 1) yRatio = 1;
        
        int x = rect.left + 50 + static_cast<int>(xRatio * chartWidth);
        int y = rect.bottom - 20 - static_cast<int>(yRatio * chartHeight);
        
        // Narysuj punkt
        HBRUSH pointBrush = CreateSolidBrush(m_dataColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, pointBrush);
        
        Ellipse(hdc, x - 3, y - 3, x + 3, y + 3);
        
        SelectObject(hdc, oldBrush);
        DeleteObject(pointBrush);
    }
}