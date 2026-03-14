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
        case WM_ERASEBKGND:
            return 1;
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
            InvalidateRect(m_hwnd, NULL, FALSE);
        }
    }
}

void Chart::addDataPoints(const double* values, int count, double totalDurationMs) {
    if (count <= 0) return;
    
    auto now = std::chrono::steady_clock::now();
    auto batchDuration = std::chrono::microseconds(static_cast<long long>(totalDurationMs * 1000.0));
    
    // Chain batches for continuity; only break on large gaps
    auto startTime = now - batchDuration;
    bool gapDetected = false;
    if (m_hasBatchHistory) {
        auto gap = std::chrono::duration_cast<std::chrono::microseconds>(now - m_lastBatchEnd);
        if (gap < batchDuration * 3) {
            startTime = m_lastBatchEnd;
        } else {
            gapDetected = true;
        }
    }
    
    auto totalSpan = std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        now - startTime);
    
    for (int i = 0; i < count; i++) {
        DataPoint point;
        point.value = values[i];
        auto offset = totalSpan * i / (count > 1 ? count - 1 : 1);
        point.timestamp = startTime + offset;
        point.isNewSegment = (i == 0 && gapDetected);
        m_dataPoints.push_back(point);
    }
    
    m_lastBatchEnd = now;
    m_hasBatchHistory = true;
    
    cleanOldDataPoints();
    m_dataChanged = true;
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastRefreshTime).count();
    if (elapsed >= m_refreshInterval) {
        m_lastRefreshTime = now;
        m_dataChanged = false;
        if (m_hwnd) {
            InvalidateRect(m_hwnd, NULL, FALSE);
        }
    }
}

void Chart::cleanOldDataPoints() {
    if (m_dataPoints.empty()) {
        return;
    }
    
    auto now = std::chrono::steady_clock::now();
    // Keep extra data when trigger is enabled (need history to search for crossings)
    double retainSec = m_triggerEnabled ? m_timeWindowSec * 3.0 : m_timeWindowSec;
    auto cutoffUs = std::chrono::microseconds(static_cast<long long>(retainSec * 1000000.0));
    auto cutoff = now - cutoffUs;
    
    // Usuń punkty starsze niż okno czasowe
    while (!m_dataPoints.empty() && m_dataPoints.front().timestamp < cutoff) {
        m_dataPoints.pop_front();
    }
}

void Chart::clear() {
    m_dataPoints.clear();
    m_hasBatchHistory = false;
    
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, FALSE);
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
    int w = clientRect.right - clientRect.left;
    int h = clientRect.bottom - clientRect.top;
    
    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    // Wypełnij tło
    HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);
    
    // Rysuj komponenty wykresu
    drawGrid(memDC, clientRect);
    drawAxes(memDC, clientRect);
    drawData(memDC, clientRect);
    
    // Rysuj tytuł
    std::wstring wideTitle = StringUtils::utf8ToWide(m_title);
    HFONT font = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
                            DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
                            CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Arial");
    
    HFONT oldFont = (HFONT)SelectObject(memDC, font);
    SetTextColor(memDC, RGB(255, 255, 255));
    SetBkMode(memDC, TRANSPARENT);
    
    RECT titleRect = clientRect;
    titleRect.bottom = 20;
    DrawTextW(memDC, wideTitle.c_str(), -1, &titleRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    
    SelectObject(memDC, oldFont);
    DeleteObject(font);
    
    // Blit to screen
    BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
    
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
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
        _snwprintf(label, 32, L"%.2f", value);
        
        RECT labelRect = {rect.left, y - 8, rect.left + 48, y + 8};
        DrawTextW(hdc, label, -1, &labelRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
    }
    
    // Etykiety osi X (czas w sekundach)
    const int verticalLines = 6;
    int stepX = (rect.right - rect.left - 60) / verticalLines;
    
    for (int i = 0; i <= verticalLines; i++) {
        int x = rect.left + 50 + i * stepX;
        double timeSec = m_timeWindowSec * (1.0 - (double)i / verticalLines);
        
        // Formatowanie etykiety czasu
        wchar_t label[32];
        if (timeSec < 0.0005)
            _snwprintf(label, 32, L"0");
        else if (m_timeWindowSec < 0.01)
            _snwprintf(label, 32, L"-%.1fms", timeSec * 1000.0);
        else if (m_timeWindowSec < 0.1)
            _snwprintf(label, 32, L"-%.0fms", timeSec * 1000.0);
        else if (m_timeWindowSec <= 5.0)
            _snwprintf(label, 32, L"-%.2fs", timeSec);
        else
            _snwprintf(label, 32, L"-%ds", (int)timeSec);
        
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
    HPEN dataPen = CreatePen(PS_SOLID, m_lineWidth, m_dataColor);
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
    
    // Czas referencyjny: najnowszy punkt danych (nie wall-clock now)
    // Dzięki temu dane są zawsze widoczne — prawy brzeg = najnowsza próbka
    auto refTime = m_dataPoints.back().timestamp;

    // Trigger mode: rising zero-crossing sync (oscilloscope-style)
    // Search backward from newest data — find the most recent crossing
    // that has at least one full time window of data after it.
    // This gives a stable display: the trigger only advances by one cycle
    // per waveform period, and stays locked between transitions.
    if (m_triggerEnabled && m_dataPoints.size() > 1) {
        auto windowUs = std::chrono::microseconds(
            static_cast<long long>(m_timeWindowSec * 1000000.0));
        auto latestTime = m_dataPoints.back().timestamp;

        for (size_t i = m_dataPoints.size() - 1; i > 0; i--) {
            if (m_dataPoints[i - 1].value <= 0.0 && m_dataPoints[i].value > 0.0) {
                auto timeAfter = std::chrono::duration_cast<std::chrono::microseconds>(
                    latestTime - m_dataPoints[i].timestamp);
                if (timeAfter >= windowUs) {
                    // Most recent eligible trigger — set left edge here
                    refTime = m_dataPoints[i].timestamp + windowUs;
                    break;
                }
            }
        }
    }
    
    // Rysowanie linii łączącej punkty danych
    bool first = true;
    for (const auto& point : m_dataPoints) {
        // Oblicz pozycję X na podstawie czasu
        auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(refTime - point.timestamp).count();
        double xRatio = 1.0 - (double)timeDiff / (m_timeWindowSec * 1000000.0);
        
        if (xRatio < 0) continue;  // Punkt poza zakresem czasu
        
        // Oblicz pozycję Y na podstawie wartości
        double yRatio = (point.value - minY) / (maxY - minY);
        if (yRatio < 0) yRatio = 0;
        if (yRatio > 1) yRatio = 1;
        
        int x = rect.left + 50 + static_cast<int>(xRatio * chartWidth);
        int y = rect.bottom - 20 - static_cast<int>(yRatio * chartHeight);
        
        if (first || point.isNewSegment) {
            MoveToEx(hdc, x, y, NULL);
            first = false;
        } else {
            LineTo(hdc, x, y);
        }
    }
    
    // Przywróć oryginalne pióro i usuń utworzone
    SelectObject(hdc, oldPen);
    DeleteObject(dataPen);
    
    // Pomijaj kropki dla gęstych danych
    if (m_dataPoints.size() > 200) return;
    
    // Narysuj punkty danych
    for (const auto& point : m_dataPoints) {
        // Oblicz pozycję X na podstawie czasu
        auto timeDiff = std::chrono::duration_cast<std::chrono::microseconds>(refTime - point.timestamp).count();
        double xRatio = 1.0 - (double)timeDiff / (m_timeWindowSec * 1000000.0);
        
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