#ifndef VALUE_DISPLAY_H
#define VALUE_DISPLAY_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <cstdint>
#include <map>
#include <functional>

// Klasa do uniwersalnego wyświetlania wartości z możliwością konfiguracji
class ValueDisplay : public UIComponent {
public:
    // Struktura konfiguracji wyświetlacza
    struct DisplayConfig {
        COLORREF backgroundColor = RGB(20, 20, 20);  // Domyślny kolor tła
        COLORREF textColor = RGB(0, 220, 0);         // Domyślny kolor tekstu
        COLORREF holdTextColor = RGB(220, 0, 0);     // Kolor dla trybu HOLD
        COLORREF deltaTextColor = RGB(0, 0, 220);    // Kolor dla trybu DELTA
        int precision = 2;                           // Liczba miejsc po przecinku
        std::wstring fontName = L"Arial";            // Nazwa czcionki
        double valueFontRatio = 0.6;                 // Współczynnik rozmiaru czcionki wartości
        double unitFontRatio = 0.25;                 // Współczynnik rozmiaru czcionki jednostek
        double statusFontRatio = 0.16;               // Współczynnik rozmiaru czcionki statusu
    };

    // Typ funkcji formatującej wartość (dla obsługi niestandardowych formatowań)
    using ValueFormatter = std::function<std::wstring(double value, int precision)>;

    ValueDisplay(int x, int y, int width, int height);
    ~ValueDisplay() override;

    void create(HWND parent) override;
    
    // Podstawowe metody aktualizacji
    void updateValue(double value, const std::wstring& prefix = L"", const std::wstring& unit = L"");
    void setMode(const std::wstring& mode);
    void setMode(uint8_t mode);  // Zachowana dla kompatybilności wstecznej
    void setRange(uint8_t range);
    
    // Metody ustawiające status
    void setAuto(bool isAuto);
    void setHold(bool isHold);
    void setDelta(bool isDelta);
    
    // Dodanie własnego statusu
    void addCustomStatus(const std::wstring& statusName, bool isActive);
    
    // Metoda do aktualizacji wszystkich parametrów na raz (kompatybilność wsteczna)
    void updateDisplay(double value, uint8_t mode, uint8_t range, 
                      bool isAuto, bool isHold, bool isDelta);

    // Nowa metoda do pełnej aktualizacji z uniwersalnymi parametrami
    void updateFullDisplay(double value, const std::wstring& prefix, const std::wstring& unit, 
                          const std::wstring& mode,
                          const std::map<std::wstring, bool>& statuses);
                          
    // Metoda do ustawienia niestandardowego formatera wartości
    void setValueFormatter(ValueFormatter formatter);
    
    // Metoda do konfiguracji wyglądu
    void setConfig(const DisplayConfig& config);
    
    // Gettery
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    HWND getHandle() const override { return m_hwnd; }
    int getId() const override { return m_id; }
    double getValue() const { return m_value; }
    uint8_t getMode() const { return m_mode; }
    uint8_t getRange() const { return m_range; }
    const std::wstring& getModeString() const { return m_modeString; }
    const std::wstring& getUnit() const { return m_unit; }
    const std::wstring& getPrefix() const { return m_prefix; }

private:
    void drawDisplay();
    COLORREF getBackgroundColor() const;
    COLORREF getTextColor() const;
    std::wstring formatValue(double value) const;
    
    // Zachowana dla kompatybilności wstecznej
    std::wstring getModeString(uint8_t mode) const;
    std::wstring getRangeString() const;

    int m_x;
    int m_y;
    int m_width;
    int m_height;
    HWND m_hwnd;
    int m_id;
    static int s_nextId;
    
    // Parametry wyświetlacza
    double m_value;                             // Aktualna wartość
    uint8_t m_mode;                             // Tryb pomiaru (dla kompatybilności wstecznej)
    uint8_t m_range;                            // Zakres pomiaru (dla kompatybilności wstecznej)
    std::wstring m_modeString;                  // Etykieta trybu (uniwersalna)
    bool m_isAuto;                              // Czy tryb auto jest aktywny
    bool m_isHold;                              // Czy funkcja hold jest aktywna
    bool m_isDelta;                             // Czy funkcja delta jest aktywna
    std::wstring m_prefix;                      // Przedrostek jednostki (m, k, M, itd.)
    std::wstring m_unit;                        // Jednostka miary (V, A, Ohm itd.)
    std::map<std::wstring, bool> m_statuses;    // Mapa statusów (nazwa, stan)
    
    // Formatowanie wartości
    ValueFormatter m_valueFormatter;
    
    // Konfiguracja wyświetlacza
    DisplayConfig m_config;
    
    // Czcionki do rysowania
    HFONT m_valueFont;
    HFONT m_unitFont;
    HFONT m_statusFont;
    
    // Procedura obsługi okna
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // VALUE_DISPLAY_H