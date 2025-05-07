#include "ValueDisplay.h"
#include "../../Util/StringUtils.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>  // Dla funkcji std::isinf()

// Inicjalizacja statycznej zmiennej
int ValueDisplay::s_nextId = 5000;

// Definicja stałych kolorów
#define COLOR_DISPLAY_BG RGB(20, 20, 20)
#define COLOR_DISPLAY_TEXT RGB(0, 220, 0)
#define COLOR_DISPLAY_TEXT_HOLD RGB(220, 0, 0)
#define COLOR_DISPLAY_TEXT_DELTA RGB(0, 0, 220)

ValueDisplay::ValueDisplay(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), m_hwnd(NULL),
      m_value(0.0), m_mode(0), m_range(0), m_modeString(L""),
      m_isAuto(false), m_isHold(false), m_isDelta(false),
      m_prefix(L""), m_unit(L""),
      m_valueFont(NULL), m_unitFont(NULL), m_statusFont(NULL) {
    m_id = s_nextId++;
    
    // Domyślny formater wartości
    m_valueFormatter = [](double value, int precision) {
        if (std::isinf(value)) {
            return std::wstring(L"OL"); // Przekroczenie zakresu pomiaru
        }
        
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(precision) << value;
        return ss.str();
    };
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
    static const wchar_t* CLASS_NAME = L"ValueDisplayClass";
    
    // Rejestracja klasy okna (tylko raz)
    static bool registered = false;
    if (!registered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = _core.hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        
        if (!RegisterClassW(&wc)) {
            MessageBoxW(NULL, L"Nie udało się zarejestrować klasy okna ValueDisplay!", L"Błąd", MB_ICONERROR);
            return;
        }
        registered = true;
    }
    
    // Utworzenie okna kontrolki
    m_hwnd = CreateWindowW(
        CLASS_NAME,
        L"",
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
void ValueDisplay::updateValue(double value, const std::wstring& prefix, const std::wstring& unit) {
    m_value = value;
    m_prefix = prefix;
    m_unit = unit;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Aktualizacja etykiety trybu pomiaru (uniwersalny)
void ValueDisplay::setMode(const std::wstring& mode) {
    m_modeString = mode;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Zachowana dla kompatybilności wstecznej
void ValueDisplay::setMode(uint8_t mode) {
    m_mode = mode;
    
    // Automatyczne ustawienie etykiety trybu na podstawie kodu
    m_modeString = getModeString(mode);
    
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
            m_unit = L"mV"; // pomiar napięcia (mV)
            break;
        case 3:
            m_unit = L"V";  // pomiar napięcia (V)
            break;
        case 5:
            m_unit = L"A";  // pomiar prądu (A)
            break;
        case 7:
            m_unit = L"mA"; // pomiar prądu (mA)
            break;
        case 9:
            m_unit = L"μA"; // pomiar prądu (mikroampery)
            break;
        case 10:
            m_unit = L"V";  // detektor napięcia
            break;
        case 11:
            m_unit = L"Ω";  // pomiar oporności
            break;
        case 12:
            m_unit = L"°C"; // pomiar temperatury
            break;
        case 13:
            m_unit = L"Hz"; // pomiar częstotliwości
            break;
        case 14:
            m_unit = L"F";  // pomiar pojemności
            break;
        default:
            m_unit = L"";
    }
    
    // Aktualizacja prefiksu na podstawie zakresu
    switch (range) {
        case 0: m_prefix = L"n"; break;     // nano
        case 1: m_prefix = L"μ"; break;     // micro
        case 2: m_prefix = L"m"; break;     // milli
        case 3: m_prefix = L""; break;      // jednostka podstawowa
        case 4: m_prefix = L"k"; break;     // kilo
        case 5: m_prefix = L"M"; break;     // mega
        default: m_prefix = L""; break;
    }
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Nowa metoda do pełnej aktualizacji z uniwersalnymi parametrami
void ValueDisplay::updateFullDisplay(double value, const std::wstring& prefix, const std::wstring& unit, 
                                    const std::wstring& mode,
                                    const std::map<std::wstring, bool>& statuses) {
    m_value = value;
    m_prefix = prefix;
    m_unit = unit;
    m_modeString = mode;
    m_statuses = statuses;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Dodanie własnego statusu
void ValueDisplay::addCustomStatus(const std::wstring& statusName, bool isActive) {
    m_statuses[statusName] = isActive;
    
    // Wymuszenie odświeżenia kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

// Metoda do ustawienia niestandardowego formatera wartości
void ValueDisplay::setValueFormatter(ValueFormatter formatter) {
    if (formatter) {
        m_valueFormatter = formatter;
        
        // Wymuszenie odświeżenia kontrolki
        if (m_hwnd) {
            InvalidateRect(m_hwnd, NULL, TRUE);
        }
    }
}

// Metoda do konfiguracji wyglądu
void ValueDisplay::setConfig(const DisplayConfig& config) {
    m_config = config;
    
    // Odtworzenie czcionek z nowymi parametrami
    if (m_hwnd) {
        // Usuń stare czcionki
        if (m_valueFont) DeleteObject(m_valueFont);
        if (m_unitFont) DeleteObject(m_unitFont);
        if (m_statusFont) DeleteObject(m_statusFont);
        
        // Utwórz nowe czcionki z nowymi parametrami
        m_valueFont = CreateFontW(
            static_cast<int>(m_height * m_config.valueFontRatio),  // Wysokość czcionki
            0, 0, 0,                        // Szerokość, pochylenie, orientacja
            FW_BOLD,                        // Grubość
            FALSE, FALSE, FALSE,            // Kursywa, podkreślenie, przekreślenie
            DEFAULT_CHARSET,                // Zestaw znaków
            OUT_DEFAULT_PRECIS,             // Precyzja wyjścia
            CLIP_DEFAULT_PRECIS,            // Precyzja przycinania
            ANTIALIASED_QUALITY,            // Jakość
            DEFAULT_PITCH | FF_SWISS,       // Typ i rodzina czcionki
            m_config.fontName.c_str()       // Nazwa czcionki
        );
        
        m_unitFont = CreateFontW(
            static_cast<int>(m_height * m_config.unitFontRatio),
            0, 0, 0,
            FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            m_config.fontName.c_str()
        );
        
        m_statusFont = CreateFontW(
            static_cast<int>(m_height * m_config.statusFontRatio),
            0, 0, 0,
            FW_NORMAL,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            m_config.fontName.c_str()
        );
        
        // Wymuszenie odświeżenia kontrolki
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
    HBRUSH bgBrush = CreateSolidBrush(getBackgroundColor());
    FillRect(memDC, &clientRect, bgBrush);
    DeleteObject(bgBrush);
    
    // Wybierz czcionkę do rysowania wartości
    HFONT oldFont = (HFONT)SelectObject(memDC, m_valueFont);
    
    // Ustaw kolor tekstu
    COLORREF textColor = getTextColor();
    SetTextColor(memDC, textColor);
    SetBkMode(memDC, TRANSPARENT);
    
    // Formatowanie wartości jako string
    std::wstring valueText = formatValue(m_value);
    
    // Obliczanie pozycji tekstu - umieszczenie na dole wyświetlacza
    SIZE valueSize;
    GetTextExtentPoint32W(memDC, valueText.c_str(), valueText.length(), &valueSize);
    int valueX = (clientRect.right - valueSize.cx) / 2;
    int valueY = clientRect.bottom - valueSize.cy - 15;  // Umieszczenie na dole z niewielkim marginesem
    
    // Rysowanie wartości
    TextOutW(memDC, valueX, valueY, valueText.c_str(), valueText.length());
    
    // Rysowanie jednostki z prefiksem (mniejszą czcionką)
    SelectObject(memDC, m_unitFont);
    std::wstring unitText = m_prefix + m_unit;
    
    SIZE unitSize;
    GetTextExtentPoint32W(memDC, unitText.c_str(), unitText.length(), &unitSize);
    int unitX = valueX + valueSize.cx + 5; // 5 pikseli odstępu
    int unitY = valueY + (valueSize.cy - unitSize.cy); // Wyrównanie do dolnej linii wartości
    
    TextOutW(memDC, unitX, unitY, unitText.c_str(), unitText.length());
    
    // Rysowanie statusu (AUTO, HOLD, DELTA oraz niestandardowe)
    SelectObject(memDC, m_statusFont);
    std::wstring statusText = L"";
    if (m_isAuto) statusText += L"AUTO ";
    if (m_isHold) statusText += L"HOLD ";
    if (m_isDelta) statusText += L"DELTA ";
    
    for (const auto& status : m_statuses) {
        if (status.second) {
            statusText += status.first + L" ";
        }
    }
    
    if (!statusText.empty()) {
        SIZE statusSize;
        GetTextExtentPoint32W(memDC, statusText.c_str(), statusText.length(), &statusSize);
        int statusX = 10; // 10 pikseli od lewej krawędzi
        int statusY = 10; // 10 pikseli od górnej krawędzi
        
        TextOutW(memDC, statusX, statusY, statusText.c_str(), statusText.length());
    }
    
    // Rysowanie trybu pomiaru
    std::wstring modeText = m_modeString.empty() ? getModeString(m_mode) : m_modeString;
    if (!modeText.empty()) {
        SIZE modeSize;
        GetTextExtentPoint32W(memDC, modeText.c_str(), modeText.length(), &modeSize);
        int modeX = clientRect.right - modeSize.cx - 10; // 10 pikseli od prawej krawędzi
        int modeY = 10; // 10 pikseli od górnej krawędzi
        
        TextOutW(memDC, modeX, modeY, modeText.c_str(), modeText.length());
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

// Metoda formatująca wartość przy użyciu formatera (lub domyślnie)
std::wstring ValueDisplay::formatValue(double value) const {
    if (m_valueFormatter) {
        return m_valueFormatter(value, m_config.precision);
    }
    
    // Domyślne formatowanie, jeśli formatter nie jest ustawiony
    if (std::isinf(value)) {
        return std::wstring(L"OL");
    }
    
    std::wstringstream ss;
    ss << std::fixed << std::setprecision(m_config.precision) << value;
    return ss.str();
}

// Konwersja trybu na tekst - wersja z parametrem mode
std::wstring ValueDisplay::getModeString(uint8_t mode) const {
    switch (mode) {
        case 1: return L"DC mV";
        case 3: return L"DC V";
        case 5: return L"DC A";
        case 7: return L"DC mA";
        case 9: return L"DC μA";
        case 10: return L"Voltage Detect";
        case 11: return L"Resistance";
        case 12: return L"Temperature";
        case 13: return L"Frequency";
        case 14: return L"Capacitance";
        default: return L"";
    }
}

// Konwersja zakresu na tekst
std::wstring ValueDisplay::getRangeString() const {
    switch (m_range) {
        case 0: return L"nano";
        case 1: return L"micro";
        case 2: return L"milli";
        case 3: return L"base";
        case 4: return L"kilo";
        case 5: return L"mega";
        default: return L"";
    }
}

// Zwraca kolor tła (z uwzględnieniem konfiguracji)
COLORREF ValueDisplay::getBackgroundColor() const {
    return m_config.backgroundColor;
}

// Zwraca kolor tekstu (z uwzględnieniem konfiguracji i statusu)
COLORREF ValueDisplay::getTextColor() const {
    if (m_isHold) {
        return m_config.holdTextColor;
    } else if (m_isDelta) {
        return m_config.deltaTextColor;
    } else {
        return m_config.textColor;
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