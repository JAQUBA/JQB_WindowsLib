#include "ValueDisplay.h"
#include <string>
#include <sstream>
#include <iomanip>

// Inicjalizacja statycznej zmiennej
int ValueDisplay::s_nextId = 5000;

// Definicja stałych kolorów
#define COLOR_DISPLAY_BG RGB(20, 20, 20)
#define COLOR_DISPLAY_TEXT RGB(0, 220, 0)
#define COLOR_DISPLAY_TEXT_HOLD RGB(220, 0, 0)
#define COLOR_DISPLAY_TEXT_DELTA RGB(0, 0, 220)

ValueDisplay::ValueDisplay(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_hwnd(NULL),
      m_value(0.0), m_mode(0), m_range(0), 
      m_isAuto(false), m_isHold(false), m_isDelta(false),
      m_prefix(""), m_unit(""),
      m_valueFont(NULL), m_unitFont(NULL), m_statusFont(NULL) {
    m_id = s_nextId++;
}

ValueDisplay::~ValueDisplay() {
    // Usunięcie utworzonych czcionek
    if (m_valueFont) {
        DeleteObject(m_valueFont);
        m_valueFont = NULL;
    }
    
    if (m_unitFont) {
        DeleteObject(m_unitFont);
        m_unitFont = NULL;
    }
    
    if (m_statusFont) {
        DeleteObject(m_statusFont);
        m_statusFont = NULL;
    }
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

// Rejestracja klasy okna i utworzenie kontrolki
void ValueDisplay::create(HWND parent) {
    // Nazwa klasy okna
    static const char* CLASS_NAME = "ValueDisplayClass";
    
    // Rejestracja klasy okna (tylko raz)
    static bool registered = false;
    if (!registered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = _core.hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        
        if (!RegisterClass(&wc)) {
            MessageBoxW(NULL, L"Nie udało się zarejestrować klasy okna ValueDisplay!", L"Błąd", MB_ICONERROR);
            return;
        }
        registered = true;
    }
    
    // Utworzenie okna kontrolki
    m_hwnd = CreateWindow(
        CLASS_NAME,
        "",
        WS_CHILD | WS_VISIBLE,
        m_x, m_y,
        m_width, m_height,
        parent,
        (HMENU)(INT_PTR)m_id,
        _core.hInstance,
        this
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć kontrolki ValueDisplay!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Ustawienie wskaźnika this w danych okna
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
    
    // Utworzenie czcionek
    m_valueFont = CreateFont(
        m_height * 3 / 5,                // Wysokość czcionki (60% wysokości kontrolki)
        0, 0, 0,                         // Szerokość, pochylenie, orientacja
        FW_BOLD,                         // Grubość
        FALSE, FALSE, FALSE,             // Kursywa, podkreślenie, przekreślenie
        DEFAULT_CHARSET,                 // Zestaw znaków
        OUT_DEFAULT_PRECIS,              // Precyzja wyjścia
        CLIP_DEFAULT_PRECIS,             // Precyzja przycinania
        ANTIALIASED_QUALITY,             // Jakość
        DEFAULT_PITCH | FF_SWISS,        // Typ i rodzina czcionki
        "Arial"                          // Nazwa czcionki
    );
    
    m_unitFont = CreateFont(
        m_height / 4,                    // Wysokość czcionki (25% wysokości kontrolki)
        0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        "Arial"
    );
    
    m_statusFont = CreateFont(
        m_height / 6,                    // Wysokość czcionki (16% wysokości kontrolki)
        0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        "Arial"
    );
}

// Aktualizacja wartości wyświetlacza
void ValueDisplay::updateValue(double value, const std::string& prefix, const std::string& unit) {
    m_value = value;
    m_prefix = prefix;
    m_unit = unit;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja trybu pomiaru
void ValueDisplay::setMode(uint8_t mode) {
    m_mode = mode;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja zakresu pomiaru
void ValueDisplay::setRange(uint8_t range) {
    m_range = range;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja flagi Auto
void ValueDisplay::setAuto(bool isAuto) {
    m_isAuto = isAuto;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja flagi Hold
void ValueDisplay::setHold(bool isHold) {
    m_isHold = isHold;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja flagi Delta
void ValueDisplay::setDelta(bool isDelta) {
    m_isDelta = isDelta;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja wszystkich parametrów naraz
void ValueDisplay::updateDisplay(double value, uint8_t mode, uint8_t range, 
                                bool isAuto, bool isHold, bool isDelta) {
    m_value = value;
    m_mode = mode;
    m_range = range;
    m_isAuto = isAuto;
    m_isHold = isHold;
    m_isDelta = isDelta;
    
    // Aktualizacja jednostki na podstawie trybu
    switch (mode) {
        case 1:
            m_unit = "mV"; // pomiar napięcia (mV)
            break;
        case 3:
            m_unit = "V";  // pomiar napięcia (V)
            break;
        case 5:
            m_unit = "A";  // pomiar prądu (A)
            break;
        case 7:
            m_unit = "mA"; // pomiar prądu (mA)
            break;
        case 9:
            m_unit = "μA"; // pomiar prądu (mikroampery)
            break;
        case 10:
            m_unit = "V";  // detektor napięcia
            break;
        case 11:
            m_unit = "Ω";  // pomiar oporności
            break;
        case 12:
            m_unit = "°C"; // pomiar temperatury
            break;
        case 13:
            m_unit = "Hz"; // pomiar częstotliwości
            break;
        case 14:
            m_unit = "F";  // pomiar pojemności
            break;
        default:
            m_unit = "";
    }
    
    // Aktualizacja prefiksu na podstawie zakresu
    switch (range) {
        case 0: m_prefix = "n"; break;     // nano
        case 1: m_prefix = "μ"; break;     // micro
        case 2: m_prefix = "m"; break;     // milli
        case 3: m_prefix = ""; break;      // jednostka podstawowa
        case 4: m_prefix = "k"; break;     // kilo
        case 5: m_prefix = "M"; break;     // mega
        default: m_prefix = ""; break;
    }
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Rysowanie wyświetlacza
void ValueDisplay::drawDisplay() {
    if (!m_hwnd) return;
    
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);
    
    // Pobierz wymiary obszaru klienta
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);
    
    // Utworzenie bufora do podwójnego buforowania
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    // Wypełnienie tła
    HBRUSH bgBrush = CreateSolidBrush(COLOR_DISPLAY_BG);
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);
    
    // Wybierz czcionkę do rysowania wartości
    HFONT oldFont = (HFONT)SelectObject(memDC, m_valueFont);
    
    // Ustaw kolor tekstu
    COLORREF textColor = getTextColor();
    SetTextColor(memDC, textColor);
    SetBkMode(memDC, TRANSPARENT);
    
    // Formatowanie wartości jako string z dwoma miejscami po przecinku
    std::stringstream ss;
    std::string valueText;
    
    // Sprawdzenie, czy wartość to nieskończoność (reprezentująca OL - Open Line)
    if (std::isinf(m_value)) {
        valueText = "OL"; // Przekroczenie zakresu pomiaru
    } else {
        ss << std::fixed << std::setprecision(2) << m_value;
        valueText = ss.str();
    }
    
    // Obliczanie pozycji tekstu
    SIZE valueSize;
    GetTextExtentPoint32A(memDC, valueText.c_str(), valueText.length(), &valueSize);
    int valueX = (clientRect.right - valueSize.cx) / 2;
    int valueY = (clientRect.bottom - valueSize.cy) / 2 - valueSize.cy / 4; // Lekko przesunięte do góry
    
    // Rysowanie wartości
    TextOutA(memDC, valueX, valueY, valueText.c_str(), valueText.length());
    
    // Rysowanie jednostki z prefiksem (mniejszą czcionką)
    SelectObject(memDC, m_unitFont);
    std::string unitText = m_prefix + m_unit;
    SIZE unitSize;
    GetTextExtentPoint32A(memDC, unitText.c_str(), unitText.length(), &unitSize);
    int unitX = valueX + valueSize.cx + 5; // 5 pikseli odstępu
    int unitY = valueY + valueSize.cy - unitSize.cy; // Wyrównanie do dolnej linii wartości
    
    TextOutA(memDC, unitX, unitY, unitText.c_str(), unitText.length());
    
    // Rysowanie statusu (AUTO, HOLD, DELTA)
    SelectObject(memDC, m_statusFont);
    std::string statusText = "";
    if (m_isAuto) statusText += "AUTO ";
    if (m_isHold) statusText += "HOLD ";
    if (m_isDelta) statusText += "DELTA ";
    
    if (!statusText.empty()) {
        SIZE statusSize;
        GetTextExtentPoint32A(memDC, statusText.c_str(), statusText.length(), &statusSize);
        int statusX = 10; // 10 pikseli od lewej krawędzi
        int statusY = 10; // 10 pikseli od górnej krawędzi
        
        TextOutA(memDC, statusX, statusY, statusText.c_str(), statusText.length());
    }
    
    // Rysowanie trybu pomiaru
    std::string modeText = getModeString();
    if (!modeText.empty()) {
        SIZE modeSize;
        GetTextExtentPoint32A(memDC, modeText.c_str(), modeText.length(), &modeSize);
        int modeX = clientRect.right - modeSize.cx - 10; // 10 pikseli od prawej krawędzi
        int modeY = 10; // 10 pikseli od górnej krawędzi
        
        TextOutA(memDC, modeX, modeY, modeText.c_str(), modeText.length());
    }
    
    // Przywróć oryginalną czcionkę
    SelectObject(memDC, oldFont);
    
    // Skopiuj zawartość bufora do rzeczywistego DC
    BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
    
    // Posprzątaj bufory
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    
    EndPaint(m_hwnd, &ps);
}

// Konwersja trybu na tekst
std::string ValueDisplay::getModeString() const {
    switch (m_mode) {
        case 1: return "DC mV";
        case 3: return "DC V";
        case 5: return "DC A";
        case 7: return "DC mA";
        case 9: return "DC μA";
        case 10: return "Voltage Detect";
        case 11: return "Resistance";
        case 12: return "Temperature";
        case 13: return "Frequency";
        case 14: return "Capacitance";
        default: return "";
    }
}

// Konwersja zakresu na tekst
std::string ValueDisplay::getRangeString() const {
    switch (m_range) {
        case 0: return "nano";
        case 1: return "micro";
        case 2: return "milli";
        case 3: return "base";
        case 4: return "kilo";
        case 5: return "mega";
        default: return "";
    }
}

// Zwraca kolor tła
COLORREF ValueDisplay::getBackgroundColor() const {
    return COLOR_DISPLAY_BG;
}

// Zwraca kolor tekstu
COLORREF ValueDisplay::getTextColor() const {
    if (m_isHold) {
        return COLOR_DISPLAY_TEXT_HOLD;
    } else if (m_isDelta) {
        return COLOR_DISPLAY_TEXT_DELTA;
    } else {
        return COLOR_DISPLAY_TEXT;
    }
}

// Procedura obsługi komunikatów okna
LRESULT CALLBACK ValueDisplay::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ValueDisplay* display = (ValueDisplay*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (uMsg) {
        case WM_PAINT:
            if (display) {
                display->drawDisplay();
                return 0;
            }
            break;
            
        case WM_ERASEBKGND:
            // Zapobieganie migotaniu
            return 1;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}