// ============================================================================
// OverlayWindow — Bazowa klasa okna nakładki (overlay)
// ============================================================================
// Osobne okno WinAPI (nie SimpleWindow!) z:
// - WS_EX_TOOLWINDOW (bez ikony na taskbarze)
// - Always-on-top (opcjonalnie)
// - Double-buffered GDI (brak migania)
// - Menu kontekstowe (kolory, pin, zamknij)
// - Zapis/odczyt pozycji i stylu z ConfigManager
//
// Podklasa nadpisuje onPaint() żeby renderować własną treść.
// ============================================================================
#ifndef OVERLAY_WINDOW_H
#define OVERLAY_WINDOW_H

#include "Core.h"
#include <string>

class ConfigManager;

class OverlayWindow {
public:
    OverlayWindow(const wchar_t* className, const wchar_t* title,
                  int defaultW = 420, int defaultH = 160);
    virtual ~OverlayWindow();

    // Otwórz / zamknij okno
    bool open(HWND parentHwnd = NULL);
    void close();
    bool isOpen() const { return m_hwnd != NULL; }
    HWND getHandle() const { return m_hwnd; }

    // Wygląd
    void setAlwaysOnTop(bool onTop);
    bool isAlwaysOnTop() const { return m_alwaysOnTop; }
    void setBackgroundColor(COLORREF color);
    void setTextColor(COLORREF color);
    COLORREF getBackgroundColor() const { return m_bgColor; }
    COLORREF getTextColor() const { return m_textColor; }

    // Minimalny rozmiar okna
    void setMinSize(int minW, int minH);

    // Automatyczny zapis/odczyt pozycji i stylu z ConfigManager
    void enablePersistence(ConfigManager& config, const std::string& prefix = "overlay");

protected:
    // Nadpisz w podklasie — rysowanie zawartości okna (double-buffered)
    virtual void onPaint(HDC memDC, const RECT& clientRect) = 0;

    // Nadpisz aby dodać pozycje do menu kontekstowego (przed domyślnymi)
    virtual void onBuildContextMenu(HMENU menu) {}

    // Nadpisz aby obsłużyć własne komendy menu (return true = obsłużone)
    virtual bool onMenuCommand(int cmdId) { return false; }

    // Wymuszenie przerysowania
    void invalidate();

    // Styl (dostępne dla podklas)
    COLORREF m_bgColor;
    COLORREF m_textColor;

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    void paintDoubleBuffered();
    void showContextMenu();
    void savePosition();
    void loadPosition();

    HWND        m_hwnd          = NULL;
    HWND        m_parentHwnd    = NULL;
    bool        m_alwaysOnTop   = true;

    int         m_posX;
    int         m_posY;
    int         m_width;
    int         m_height;
    int         m_minWidth      = 200;
    int         m_minHeight     = 80;

    const wchar_t* m_className;
    std::wstring   m_title;

    // Persistence (opcjonalne)
    ConfigManager* m_config       = nullptr;
    std::string    m_configPrefix;
};

// ID menu kontekstowego — bazowe (9100–9149), podklasy: 9150+
#define IDM_OVL_ALWAYS_ON_TOP  9100
#define IDM_OVL_BG_BLACK       9101
#define IDM_OVL_BG_GREEN       9102
#define IDM_OVL_BG_BLUE        9103
#define IDM_OVL_BG_MAGENTA     9104
#define IDM_OVL_TEXT_GREEN      9110
#define IDM_OVL_TEXT_WHITE      9111
#define IDM_OVL_TEXT_YELLOW     9112
#define IDM_OVL_TEXT_CYAN       9113
#define IDM_OVL_CLOSE          9199

#endif // OVERLAY_WINDOW_H
