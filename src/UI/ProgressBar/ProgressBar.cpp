#include "ProgressBar.h"
#include <CommCtrl.h>
#include <WindowsX.h>

/* Fallback defines for MinGW.org that lacks PBS_MARQUEE / PBM_SETMARQUEE */
#ifndef PBS_MARQUEE
#define PBS_MARQUEE 0x08
#endif
#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE (WM_USER + 10)
#endif

// Inicjalizacja zmiennej statycznej
int ProgressBar::s_nextId = 6000;

ProgressBar::ProgressBar(int x, int y, int width, int height, int minimum, int maximum)
    : m_x(x), m_y(y), m_width(width), m_height(height), 
      m_minimum(minimum), m_maximum(maximum), m_value(minimum),
      m_color(RGB(0, 120, 215)), m_backColor(RGB(240, 240, 240)),
      m_isMarquee(false), m_marqueeInterval(30), m_hwnd(NULL) {
    m_id = s_nextId++;
    
    // Upewnij się, że minimum < maximum
    if (m_minimum >= m_maximum) {
        m_minimum = 0;
        m_maximum = 100;
    }
}

ProgressBar::~ProgressBar() {
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void ProgressBar::create(HWND parent) {
    // Upewnij się, że kontrolki Common Controls są zainicjowane
    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icc);
    
    // Utworzenie paska postępu
    m_hwnd = CreateWindowExW(
        0,                          // Rozszerzony styl okna
        PROGRESS_CLASSW,            // Nazwa klasy
        NULL,                       // Tekst
        WS_CHILD | WS_VISIBLE,      // Styl okna
        m_x, m_y,                   // Pozycja
        m_width, m_height,          // Rozmiar
        parent,                     // Uchwyt rodzica
        (HMENU)(INT_PTR)m_id,       // ID menu
        _core.hInstance,            // Instancja
        NULL                        // Dane użytkownika
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć paska postępu!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Ustaw zakres i wartość początkową
    SendMessage(m_hwnd, PBM_SETRANGE32, m_minimum, m_maximum);
    SendMessage(m_hwnd, PBM_SETPOS, m_value, 0);
    
    // Ustaw kolory, jeśli zostały zmienione
    if (m_color != RGB(0, 120, 215)) {
        SendMessage(m_hwnd, PBM_SETBARCOLOR, 0, (LPARAM)m_color);
    }
    
    if (m_backColor != RGB(240, 240, 240)) {
        SendMessage(m_hwnd, PBM_SETBKCOLOR, 0, (LPARAM)m_backColor);
    }
    
    // Ustaw tryb Marquee, jeśli jest włączony
    if (m_isMarquee) {
        DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
        style |= PBS_MARQUEE;
        SetWindowLong(m_hwnd, GWL_STYLE, style);
        SendMessage(m_hwnd, PBM_SETMARQUEE, TRUE, m_marqueeInterval);
    }
}

void ProgressBar::setProgress(int value) {
    // Ogranicz wartość do zakresu
    if (value < m_minimum) {
        value = m_minimum;
    } else if (value > m_maximum) {
        value = m_maximum;
    }
    
    m_value = value;
    
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETPOS, m_value, 0);
    }
}

int ProgressBar::getProgress() const {
    if (m_hwnd) {
        return (int)SendMessage(m_hwnd, PBM_GETPOS, 0, 0);
    }
    return m_value;
}

void ProgressBar::setRange(int minimum, int maximum) {
    // Upewnij się, że minimum < maximum
    if (minimum >= maximum) {
        return;
    }
    
    m_minimum = minimum;
    m_maximum = maximum;
    
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETRANGE32, m_minimum, m_maximum);
        
        // Dostosuj aktualną wartość, jeśli jest poza nowym zakresem
        if (m_value < m_minimum) {
            m_value = m_minimum;
            SendMessage(m_hwnd, PBM_SETPOS, m_value, 0);
        } else if (m_value > m_maximum) {
            m_value = m_maximum;
            SendMessage(m_hwnd, PBM_SETPOS, m_value, 0);
        }
    }
}

void ProgressBar::getRange(int& minimum, int& maximum) const {
    minimum = m_minimum;
    maximum = m_maximum;
}

void ProgressBar::setColor(COLORREF color) {
    m_color = color;
    
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETBARCOLOR, 0, (LPARAM)m_color);
    }
}

void ProgressBar::setBackColor(COLORREF color) {
    m_backColor = color;
    
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_SETBKCOLOR, 0, (LPARAM)m_backColor);
    }
}

void ProgressBar::setMarquee(bool enable, int interval) {
    m_isMarquee = enable;
    m_marqueeInterval = interval;
    
    if (m_hwnd) {
        DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
        
        if (m_isMarquee) {
            // Włącz tryb Marquee
            style |= PBS_MARQUEE;
            SetWindowLong(m_hwnd, GWL_STYLE, style);
            SendMessage(m_hwnd, PBM_SETMARQUEE, TRUE, m_marqueeInterval);
        } else {
            // Wyłącz tryb Marquee
            style &= ~PBS_MARQUEE;
            SetWindowLong(m_hwnd, GWL_STYLE, style);
            SendMessage(m_hwnd, PBM_SETMARQUEE, FALSE, m_marqueeInterval);
            
            // Przywróć poprzednią wartość
            SendMessage(m_hwnd, PBM_SETPOS, m_value, 0);
        }
    }
}

void ProgressBar::step(int increment) {
    if (m_hwnd) {
        SendMessage(m_hwnd, PBM_DELTAPOS, increment, 0);
        
        // Aktualizuj przechowywaną wartość
        m_value = getProgress();
    } else {
        setProgress(m_value + increment);
    }
}