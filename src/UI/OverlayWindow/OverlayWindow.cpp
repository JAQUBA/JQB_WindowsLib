// ============================================================================
// OverlayWindow — Bazowa klasa okna nakładki (overlay)
// ============================================================================
#include "OverlayWindow.h"
#include "../../Util/ConfigManager.h"

#include <cstdio>

// ============================================================================
// Konstruktor / Destruktor
// ============================================================================
OverlayWindow::OverlayWindow(const wchar_t* className, const wchar_t* title,
                             int defaultW, int defaultH)
    : m_className(className)
    , m_title(title)
    , m_bgColor(RGB(0, 0, 0))
    , m_textColor(RGB(0, 255, 0))
    , m_posX(100)
    , m_posY(100)
    , m_width(defaultW)
    , m_height(defaultH)
{
}

OverlayWindow::~OverlayWindow() {
    close();
}

// ============================================================================
// Otwórz okno
// ============================================================================
bool OverlayWindow::open(HWND parentHwnd) {
    if (m_hwnd) return true; // Już otwarte

    m_parentHwnd = parentHwnd;

    // Rejestracja klasy okna (ignoruj jeśli już zarejestrowana)
    WNDCLASSW wc = {};
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = _core.hInstance;
    wc.lpszClassName  = m_className;
    wc.hCursor        = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.hbrBackground  = NULL; // Rysujemy sami
    wc.style          = CS_HREDRAW | CS_VREDRAW;
    RegisterClassW(&wc); // OK jeśli już zarejestrowana

    // Wczytaj zapisaną pozycję (jeśli persistence włączone)
    loadPosition();

    DWORD exStyle = WS_EX_TOOLWINDOW; // Bez ikony na taskbarze
    if (m_alwaysOnTop) exStyle |= WS_EX_TOPMOST;

    m_hwnd = CreateWindowExW(
        exStyle,
        m_className,
        m_title.c_str(),
        WS_POPUP | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU,
        m_posX, m_posY, m_width, m_height,
        NULL, // Niezależne okno
        NULL,
        _core.hInstance,
        this  // Przekaż pointer w CREATESTRUCT
    );

    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    return true;
}

// ============================================================================
// Zamknij okno
// ============================================================================
void OverlayWindow::close() {
    if (m_hwnd) {
        savePosition();
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

// ============================================================================
// Ustawienia wyglądu
// ============================================================================
void OverlayWindow::setAlwaysOnTop(bool onTop) {
    m_alwaysOnTop = onTop;
    if (m_hwnd) {
        SetWindowPos(m_hwnd, onTop ? HWND_TOPMOST : HWND_NOTOPMOST,
                     0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
}

void OverlayWindow::setBackgroundColor(COLORREF color) {
    m_bgColor = color;
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

void OverlayWindow::setTextColor(COLORREF color) {
    m_textColor = color;
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

void OverlayWindow::setMinSize(int minW, int minH) {
    m_minWidth = minW;
    m_minHeight = minH;
}

void OverlayWindow::enablePersistence(ConfigManager& config, const std::string& prefix) {
    m_config = &config;
    m_configPrefix = prefix;
}

void OverlayWindow::invalidate() {
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, FALSE);
}

// ============================================================================
// Zapis/odczyt pozycji okna
// ============================================================================
void OverlayWindow::savePosition() {
    if (!m_hwnd || !m_config) return;
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    m_posX   = rc.left;
    m_posY   = rc.top;
    m_width  = rc.right - rc.left;
    m_height = rc.bottom - rc.top;

    m_config->setValue(m_configPrefix + "_x",      std::to_string(m_posX));
    m_config->setValue(m_configPrefix + "_y",      std::to_string(m_posY));
    m_config->setValue(m_configPrefix + "_w",      std::to_string(m_width));
    m_config->setValue(m_configPrefix + "_h",      std::to_string(m_height));
    m_config->setValue(m_configPrefix + "_ontop",  m_alwaysOnTop ? "1" : "0");
    m_config->setValue(m_configPrefix + "_bg",     std::to_string(m_bgColor));
    m_config->setValue(m_configPrefix + "_text",   std::to_string(m_textColor));
}

void OverlayWindow::loadPosition() {
    if (!m_config) return;

    std::string sx = m_config->getValue(m_configPrefix + "_x", std::to_string(m_posX));
    std::string sy = m_config->getValue(m_configPrefix + "_y", std::to_string(m_posY));
    std::string sw = m_config->getValue(m_configPrefix + "_w", std::to_string(m_width));
    std::string sh = m_config->getValue(m_configPrefix + "_h", std::to_string(m_height));
    m_posX   = std::stoi(sx);
    m_posY   = std::stoi(sy);
    m_width  = std::stoi(sw);
    m_height = std::stoi(sh);
    m_alwaysOnTop = m_config->getValue(m_configPrefix + "_ontop", "1") == "1";

    std::string bg = m_config->getValue(m_configPrefix + "_bg", "");
    std::string tx = m_config->getValue(m_configPrefix + "_text", "");
    if (!bg.empty()) m_bgColor   = (COLORREF)std::stoul(bg);
    if (!tx.empty()) m_textColor = (COLORREF)std::stoul(tx);
}

// ============================================================================
// WindowProc
// ============================================================================
LRESULT CALLBACK OverlayWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    OverlayWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lp);
        self = reinterpret_cast<OverlayWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<OverlayWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
        case WM_PAINT:
            self->paintDoubleBuffered();
            return 0;

        case WM_ERASEBKGND:
            return 1; // Zapobiegaj miganiu — rysujemy tło w WM_PAINT

        case WM_RBUTTONUP:
            self->showContextMenu();
            return 0;

        case WM_COMMAND: {
            int cmdId = LOWORD(wp);
            // Najpierw daj podklasie szansę obsłużyć
            if (self->onMenuCommand(cmdId)) return 0;
            // Domyślne komendy bazowe
            switch (cmdId) {
                case IDM_OVL_ALWAYS_ON_TOP:
                    self->setAlwaysOnTop(!self->m_alwaysOnTop);
                    break;
                case IDM_OVL_BG_BLACK:
                    self->setBackgroundColor(RGB(0, 0, 0));
                    break;
                case IDM_OVL_BG_GREEN:
                    self->setBackgroundColor(RGB(0, 177, 64));
                    break;
                case IDM_OVL_BG_BLUE:
                    self->setBackgroundColor(RGB(0, 0, 255));
                    break;
                case IDM_OVL_BG_MAGENTA:
                    self->setBackgroundColor(RGB(255, 0, 255));
                    break;
                case IDM_OVL_TEXT_GREEN:
                    self->setTextColor(RGB(0, 255, 0));
                    break;
                case IDM_OVL_TEXT_WHITE:
                    self->setTextColor(RGB(255, 255, 255));
                    break;
                case IDM_OVL_TEXT_YELLOW:
                    self->setTextColor(RGB(255, 255, 0));
                    break;
                case IDM_OVL_TEXT_CYAN:
                    self->setTextColor(RGB(0, 255, 255));
                    break;
                case IDM_OVL_CLOSE:
                    self->close();
                    return 0;
            }
            break;
        }

        case WM_DESTROY:
            self->savePosition();
            self->m_hwnd = NULL;
            return 0;

        case WM_GETMINMAXINFO: {
            auto mmi = reinterpret_cast<MINMAXINFO*>(lp);
            mmi->ptMinTrackSize.x = self->m_minWidth;
            mmi->ptMinTrackSize.y = self->m_minHeight;
            return 0;
        }
    }

    return DefWindowProcW(hwnd, msg, wp, lp);
}

// ============================================================================
// Rysowanie (double-buffered GDI)
// ============================================================================
void OverlayWindow::paintDoubleBuffered() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int cw = rc.right;
    int ch = rc.bottom;

    // Double buffering
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, cw, ch);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    // Tło
    HBRUSH bgBrush = CreateSolidBrush(m_bgColor);
    FillRect(memDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    SetBkMode(memDC, TRANSPARENT);

    // Wywołaj podklasę
    onPaint(memDC, rc);

    // Blit
    BitBlt(hdc, 0, 0, cw, ch, memDC, 0, 0, SRCCOPY);

    // Cleanup
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(m_hwnd, &ps);
}

// ============================================================================
// Menu kontekstowe (prawy klik)
// ============================================================================
void OverlayWindow::showContextMenu() {
    HMENU menu = CreatePopupMenu();

    // Pozycje z podklasy (przed domyślnymi)
    onBuildContextMenu(menu);

    // Domyślne pozycje
    AppendMenuW(menu, MF_STRING | (m_alwaysOnTop ? MF_CHECKED : MF_UNCHECKED),
                IDM_OVL_ALWAYS_ON_TOP, L"Zawsze na wierzchu");

    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);

    // Kolor tła
    HMENU bgMenu = CreatePopupMenu();
    AppendMenuW(bgMenu, MF_STRING, IDM_OVL_BG_BLACK,   L"Czarny");
    AppendMenuW(bgMenu, MF_STRING, IDM_OVL_BG_GREEN,   L"Zielony (chroma key)");
    AppendMenuW(bgMenu, MF_STRING, IDM_OVL_BG_BLUE,    L"Niebieski (chroma key)");
    AppendMenuW(bgMenu, MF_STRING, IDM_OVL_BG_MAGENTA, L"Magenta (chroma key)");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)bgMenu, L"Kolor t\x0142a");

    // Kolor tekstu
    HMENU txMenu = CreatePopupMenu();
    AppendMenuW(txMenu, MF_STRING, IDM_OVL_TEXT_GREEN,  L"Zielony");
    AppendMenuW(txMenu, MF_STRING, IDM_OVL_TEXT_WHITE,  L"Bia\x0142y");
    AppendMenuW(txMenu, MF_STRING, IDM_OVL_TEXT_YELLOW, L"\x017B\x00F3\x0142ty");
    AppendMenuW(txMenu, MF_STRING, IDM_OVL_TEXT_CYAN,   L"Cyjan");
    AppendMenuW(menu, MF_POPUP, (UINT_PTR)txMenu, L"Kolor tekstu");

    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, IDM_OVL_CLOSE, L"Zamknij overlay");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(m_hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, NULL);
    DestroyMenu(menu);
}
