#ifndef VALUE_DISPLAY_H
#define VALUE_DISPLAY_H

#include "Core.h"
#include <string>
#include <cstdint>

// Klasa do wyświetlania wartości z multimetru
class ValueDisplay {
public:
    ValueDisplay(int x, int y, int width, int height);
    ~ValueDisplay();

    void create(HWND parent);
    void updateValue(double value, const std::string& prefix, const std::string& unit);
    void setMode(uint8_t mode);
    void setRange(uint8_t range);
    void setAuto(bool isAuto);
    void setHold(bool isHold);
    void setDelta(bool isDelta);
    
    // Metoda do aktualizacji wszystkich parametrów na raz
    void updateDisplay(double value, uint8_t mode, uint8_t range, 
                      bool isAuto, bool isHold, bool isDelta);

    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    HWND getHandle() const { return m_hwnd; }
    int getId() const { return m_id; }
    double getValue() const { return m_value; }
    uint8_t getMode() const { return m_mode; }
    uint8_t getRange() const { return m_range; }

private:
    void drawDisplay();
    std::string getModeString() const;
    std::string getRangeString() const;
    COLORREF getBackgroundColor() const;
    COLORREF getTextColor() const;

    int m_x;
    int m_y;
    int m_width;
    int m_height;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
    
    // Parametry wyświetlacza
    double m_value;           // Aktualna wartość
    uint8_t m_mode;           // Tryb pomiaru (V, A, Ohm itd.)
    uint8_t m_range;          // Zakres pomiaru
    bool m_isAuto;            // Czy tryb auto jest aktywny
    bool m_isHold;            // Czy funkcja hold jest aktywna
    bool m_isDelta;           // Czy funkcja delta jest aktywna
    std::string m_prefix;     // Przedrostek jednostki (m, k, M, itd.)
    std::string m_unit;       // Jednostka miary (V, A, Ohm itd.)
    
    // Czcionka do rysowania wartości
    HFONT m_valueFont;
    HFONT m_unitFont;
    HFONT m_statusFont;
    
    // Procedura obsługi okna
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // VALUE_DISPLAY_H