// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

/*
 * KeyCapture.cpp — Owner-draw key-capture button implementation
 */
#include "KeyCapture.h"
#include "../../Util/StringUtils.h"
#include <commctrl.h>
#include <cstdio>

/* ================================================================== */
/*  Static members                                                     */
/* ================================================================== */

int KeyCapture::s_nextId = 8500;

/* ================================================================== */
/*  Utility functions                                                  */
/* ================================================================== */

std::string vkToName(UINT vk) {
    if (vk == 0) return "(none)";
    if (vk >= 'A' && vk <= 'Z') { char s[2] = {(char)vk, 0}; return s; }
    if (vk >= '0' && vk <= '9') { char s[2] = {(char)vk, 0}; return s; }
    switch (vk) {
        case VK_SPACE:      return "Space";
        case VK_RETURN:     return "Enter";
        case VK_ESCAPE:     return "Esc";
        case VK_BACK:       return "Backspace";
        case VK_TAB:        return "Tab";
        case VK_UP:         return "Up";
        case VK_DOWN:       return "Down";
        case VK_LEFT:       return "Left";
        case VK_RIGHT:      return "Right";
        case VK_INSERT:     return "Insert";
        case VK_DELETE:     return "Delete";
        case VK_HOME:       return "Home";
        case VK_END:        return "End";
        case VK_PRIOR:      return "PageUp";
        case VK_NEXT:       return "PageDown";
        case VK_CAPITAL:    return "CapsLock";
        case VK_NUMLOCK:    return "NumLock";
        case VK_SCROLL:     return "ScrollLock";
        case VK_SNAPSHOT:   return "PrintScreen";
        case VK_PAUSE:      return "Pause";
        case VK_LCONTROL:   return "LCtrl";
        case VK_RCONTROL:   return "RCtrl";
        case VK_LSHIFT:     return "LShift";
        case VK_RSHIFT:     return "RShift";
        case VK_LMENU:      return "LAlt";
        case VK_RMENU:      return "RAlt";
        case VK_LWIN:       return "LWin";
        case VK_RWIN:       return "RWin";
        case VK_F1:  return "F1";   case VK_F2:  return "F2";
        case VK_F3:  return "F3";   case VK_F4:  return "F4";
        case VK_F5:  return "F5";   case VK_F6:  return "F6";
        case VK_F7:  return "F7";   case VK_F8:  return "F8";
        case VK_F9:  return "F9";   case VK_F10: return "F10";
        case VK_F11: return "F11";  case VK_F12: return "F12";
        case VK_F13: return "F13";  case VK_F14: return "F14";
        case VK_F15: return "F15";  case VK_F16: return "F16";
        case VK_F17: return "F17";  case VK_F18: return "F18";
        case VK_F19: return "F19";  case VK_F20: return "F20";
        case VK_F21: return "F21";  case VK_F22: return "F22";
        case VK_F23: return "F23";  case VK_F24: return "F24";
        case VK_NUMPAD0: return "Num0"; case VK_NUMPAD1: return "Num1";
        case VK_NUMPAD2: return "Num2"; case VK_NUMPAD3: return "Num3";
        case VK_NUMPAD4: return "Num4"; case VK_NUMPAD5: return "Num5";
        case VK_NUMPAD6: return "Num6"; case VK_NUMPAD7: return "Num7";
        case VK_NUMPAD8: return "Num8"; case VK_NUMPAD9: return "Num9";
        case VK_MULTIPLY: return "Num*";
        case VK_ADD:      return "Num+";
        case VK_SUBTRACT: return "Num-";
        case VK_DECIMAL:  return "Num.";
        case VK_DIVIDE:   return "Num/";
        case VK_OEM_MINUS:  return "-";
        case VK_OEM_PLUS:   return "=";
        case VK_OEM_4:      return "[";
        case VK_OEM_6:      return "]";
        case VK_OEM_1:      return ";";
        case VK_OEM_7:      return "'";
        case VK_OEM_COMMA:  return ",";
        case VK_OEM_PERIOD: return ".";
        case VK_OEM_2:      return "/";
        case VK_OEM_5:      return "\\";
        case VK_OEM_3:      return "`";
        case VK_VOLUME_MUTE:       return "Mute";
        case VK_VOLUME_DOWN:       return "Vol-";
        case VK_VOLUME_UP:         return "Vol+";
        case VK_MEDIA_NEXT_TRACK:  return "Next";
        case VK_MEDIA_PREV_TRACK:  return "Prev";
        case VK_MEDIA_STOP:        return "Stop";
        case VK_MEDIA_PLAY_PAUSE:  return "Play/Pause";
        default: {
            char buf[8];
            snprintf(buf, sizeof(buf), "0x%02X", vk);
            return buf;
        }
    }
}

std::string modBitsToString(uint8_t bits) {
    std::string s;
    if (bits & KCMOD_LCTRL)  s += "LCtrl+";
    if (bits & KCMOD_RCTRL)  s += "RCtrl+";
    if (bits & KCMOD_LSHIFT) s += "LShift+";
    if (bits & KCMOD_RSHIFT) s += "RShift+";
    if (bits & KCMOD_LALT)   s += "LAlt+";
    if (bits & KCMOD_RALT)   s += "RAlt+";
    if (bits & KCMOD_LGUI)   s += "LWin+";
    if (bits & KCMOD_RGUI)   s += "RWin+";
    return s;
}

std::string comboToString(const KeyCombo& combo) {
    return modBitsToString(combo.modifiers) + vkToName(combo.vkCode);
}

uint8_t getAsyncModifiers() {
    uint8_t bits = 0;
    if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) bits |= KCMOD_LCTRL;
    if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) bits |= KCMOD_RCTRL;
    if (GetAsyncKeyState(VK_LSHIFT)   & 0x8000) bits |= KCMOD_LSHIFT;
    if (GetAsyncKeyState(VK_RSHIFT)   & 0x8000) bits |= KCMOD_RSHIFT;
    if (GetAsyncKeyState(VK_LMENU)    & 0x8000) bits |= KCMOD_LALT;
    if (GetAsyncKeyState(VK_RMENU)    & 0x8000) bits |= KCMOD_RALT;
    if (GetAsyncKeyState(VK_LWIN)     & 0x8000) bits |= KCMOD_LGUI;
    if (GetAsyncKeyState(VK_RWIN)     & 0x8000) bits |= KCMOD_RGUI;
    return bits;
}

bool isModifierVk(UINT vk) {
    return vk == VK_LCONTROL || vk == VK_RCONTROL ||
           vk == VK_LSHIFT   || vk == VK_RSHIFT   ||
           vk == VK_LMENU    || vk == VK_RMENU    ||
           vk == VK_LWIN     || vk == VK_RWIN;
}

uint8_t vkToModBit(UINT vk) {
    switch (vk) {
        case VK_LCONTROL: return KCMOD_LCTRL;
        case VK_RCONTROL: return KCMOD_RCTRL;
        case VK_LSHIFT:   return KCMOD_LSHIFT;
        case VK_RSHIFT:   return KCMOD_RSHIFT;
        case VK_LMENU:    return KCMOD_LALT;
        case VK_RMENU:    return KCMOD_RALT;
        case VK_LWIN:     return KCMOD_LGUI;
        case VK_RWIN:     return KCMOD_RGUI;
        default:          return 0;
    }
}

/* ================================================================== */
/*  Resolve generic VK_SHIFT / VK_CONTROL / VK_MENU to L/R variant   */
/* ================================================================== */

static UINT resolveModifierVk(UINT vk, LPARAM lp) {
    if (vk == VK_SHIFT)
        return (MapVirtualKeyW((lp >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX)
                == VK_RSHIFT) ? VK_RSHIFT : VK_LSHIFT;
    if (vk == VK_CONTROL)
        return (lp & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL;
    if (vk == VK_MENU)
        return (lp & (1 << 24)) ? VK_RMENU : VK_LMENU;
    return vk;
}

/* ================================================================== */
/*  KeyCapture — construction / destruction                            */
/* ================================================================== */

KeyCapture::KeyCapture(int x, int y, int w, int h,
                       std::function<void(KeyCapture*, const KeyCombo&)> onChange)
    : m_x(x), m_y(y), m_w(w), m_h(h), m_onChange(onChange) {
    m_id = s_nextId++;
}

KeyCapture::~KeyCapture() {
    if (m_hFont) { DeleteObject(m_hFont); m_hFont = NULL; }
    if (m_hwnd) {
        RemoveWindowSubclass(m_hwnd, SubclassProc, 0);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

/* ================================================================== */
/*  create                                                             */
/* ================================================================== */

void KeyCapture::create(HWND parent) {
    std::string text = getDisplayText();
    std::wstring wt = StringUtils::utf8ToWide(text);

    m_hwnd = CreateWindowExW(0, L"BUTTON", wt.c_str(),
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW | WS_TABSTOP,
        m_x, m_y, m_w, m_h, parent, (HMENU)(INT_PTR)m_id,
        _core.hInstance, NULL);

    if (!m_hwnd) return;

    if (!m_hFont) {
        m_hFont = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    }

    SetWindowSubclass(m_hwnd, SubclassProc, 0, (DWORD_PTR)this);
}

/* ================================================================== */
/*  setCombo                                                           */
/* ================================================================== */

void KeyCapture::setCombo(const KeyCombo& combo) {
    m_combo     = combo;
    m_listening = false;
    if (m_hwnd) {
        std::wstring wt = StringUtils::utf8ToWide(getDisplayText());
        SetWindowTextW(m_hwnd, wt.c_str());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

void KeyCapture::setCombo(uint8_t modifiers, UINT vkCode) {
    setCombo({vkCode, modifiers});
}

/* ================================================================== */
/*  Internal helpers                                                   */
/* ================================================================== */

std::string KeyCapture::getDisplayText() const {
    if (m_comboFormatter) return m_comboFormatter(m_combo);
    return comboToString(m_combo);
}

void KeyCapture::commitCombo(uint8_t mod, UINT vk) {
    m_combo.modifiers = mod;
    m_combo.vkCode    = vk;
    m_listening       = false;
    if (m_hwnd) {
        std::wstring wt = StringUtils::utf8ToWide(getDisplayText());
        SetWindowTextW(m_hwnd, wt.c_str());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
    if (m_onChange) m_onChange(this, m_combo);
}

void KeyCapture::cancelCapture() {
    m_listening = false;
    if (m_hwnd) {
        std::wstring wt = StringUtils::utf8ToWide(getDisplayText());
        SetWindowTextW(m_hwnd, wt.c_str());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

/* ================================================================== */
/*  Owner-draw rendering                                               */
/* ================================================================== */

bool KeyCapture::handleDrawItem(DRAWITEMSTRUCT* dis) {
    if ((int)dis->CtlID != m_id) return false;

    int saved = SaveDC(dis->hDC);
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;

    /* Choose background color */
    COLORREF bg;
    if (m_listening) {
        COLORREF active = (m_activeColor != CLR_INVALID)
                          ? m_activeColor : RGB(137, 180, 250);
        bg = pressed ? RGB(GetRValue(active) * 3 / 4,
                           GetGValue(active) * 3 / 4,
                           GetBValue(active) * 3 / 4)
                     : active;
    } else {
        COLORREF normal = (m_backColor != CLR_INVALID)
                          ? m_backColor : GetSysColor(COLOR_BTNFACE);
        bg = pressed ? RGB(GetRValue(normal) * 3 / 4,
                           GetGValue(normal) * 3 / 4,
                           GetBValue(normal) * 3 / 4)
                     : normal;
    }

    /* Choose border color */
    COLORREF border;
    if (m_listening)
        border = (m_activeBorderClr != CLR_INVALID) ? m_activeBorderClr
                 : RGB(137, 180, 250);
    else
        border = (m_borderColor != CLR_INVALID) ? m_borderColor
                 : RGB(49, 50, 68);

    /* Draw rounded rectangle */
    HBRUSH hBrush = CreateSolidBrush(bg);
    HPEN   hPen   = CreatePen(PS_SOLID, 1, border);
    HBRUSH oldB = (HBRUSH)SelectObject(dis->hDC, hBrush);
    HPEN   oldP = (HPEN)SelectObject(dis->hDC, hPen);
    RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top,
              dis->rcItem.right, dis->rcItem.bottom, 8, 8);
    SelectObject(dis->hDC, oldB);
    SelectObject(dis->hDC, oldP);
    DeleteObject(hBrush);
    DeleteObject(hPen);

    /* Draw text */
    if (m_hFont) SelectObject(dis->hDC, m_hFont);

    COLORREF textClr;
    if (m_listening)
        textClr = RGB(24, 24, 37); /* dark text on bright active bg */
    else
        textClr = (m_textColor != CLR_INVALID) ? m_textColor
                  : GetSysColor(COLOR_BTNTEXT);

    SetTextColor(dis->hDC, textClr);
    SetBkMode(dis->hDC, TRANSPARENT);
    wchar_t text[128];
    GetWindowTextW(dis->hwndItem, text, 128);
    DrawTextW(dis->hDC, text, -1, &dis->rcItem,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    RestoreDC(dis->hDC, saved);
    return true;
}

/* ================================================================== */
/*  Styling setters                                                    */
/* ================================================================== */

void KeyCapture::setBackColor(COLORREF color)         { m_backColor = color; }
void KeyCapture::setTextColor(COLORREF color)         { m_textColor = color; }
void KeyCapture::setActiveColor(COLORREF color)       { m_activeColor = color; }
void KeyCapture::setActiveBorderColor(COLORREF color) { m_activeBorderClr = color; }
void KeyCapture::setBorderColor(COLORREF color)       { m_borderColor = color; }

void KeyCapture::setFont(const wchar_t* fontName, int size, bool bold) {
    if (m_hFont) DeleteObject(m_hFont);
    m_hFont = CreateFontW(
        -size, 0, 0, 0,
        bold ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        fontName);
    if (m_hwnd) InvalidateRect(m_hwnd, NULL, TRUE);
}

void KeyCapture::setComboFormatter(
        std::function<std::string(const KeyCombo&)> fmt) {
    m_comboFormatter = fmt;
    if (m_hwnd && !m_listening) {
        std::wstring wt = StringUtils::utf8ToWide(getDisplayText());
        SetWindowTextW(m_hwnd, wt.c_str());
        InvalidateRect(m_hwnd, NULL, TRUE);
    }
}

/* ================================================================== */
/*  Subclass WndProc — key capture logic                               */
/* ================================================================== */

LRESULT CALLBACK KeyCapture::SubclassProc(HWND hwnd, UINT msg, WPARAM wp,
                                          LPARAM lp, UINT_PTR idSubclass,
                                          DWORD_PTR refData) {
    KeyCapture* kc = (KeyCapture*)refData;

    switch (msg) {

    /* ── Click → start listening ─────────────────────────────── */
    case WM_LBUTTONDOWN:
        kc->m_listening  = true;
        kc->m_pendingMod = 0;
        kc->m_lastModVk  = 0;
        SetWindowTextW(hwnd, L"...");
        InvalidateRect(hwnd, NULL, TRUE);
        SetFocus(hwnd);
        return 0;

    /* ── Key down ────────────────────────────────────────────── */
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        if (kc->m_listening) {
            UINT vk = resolveModifierVk((UINT)wp, lp);

            if (isModifierVk(vk)) {
                /* Modifier pressed — update preview, keep listening */
                kc->m_pendingMod = getAsyncModifiers();
                kc->m_lastModVk  = vk;
                std::string preview = modBitsToString(kc->m_pendingMod) + "...";
                std::wstring wp2 = StringUtils::utf8ToWide(preview);
                SetWindowTextW(hwnd, wp2.c_str());
                InvalidateRect(hwnd, NULL, TRUE);
            } else {
                /* Non-modifier pressed — commit combo */
                kc->commitCombo(getAsyncModifiers(), vk);
            }
            return 0;
        }
        break;

    /* ── Key up ──────────────────────────────────────────────── */
    case WM_KEYUP:
    case WM_SYSKEYUP:
        if (kc->m_listening) {
            uint8_t nowHeld = getAsyncModifiers();
            if (nowHeld == 0 && kc->m_lastModVk != 0) {
                /* All modifiers released — commit standalone modifier */
                uint8_t keyBit  = vkToModBit(kc->m_lastModVk);
                uint8_t modBits = kc->m_pendingMod & ~keyBit;
                kc->commitCombo(modBits, kc->m_lastModVk);
            } else {
                /* Some modifiers still held — update preview */
                kc->m_pendingMod = nowHeld;
                std::string preview = modBitsToString(nowHeld) + "...";
                std::wstring wp2 = StringUtils::utf8ToWide(preview);
                SetWindowTextW(hwnd, wp2.c_str());
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        }
        break;

    /* ── Focus lost → cancel ─────────────────────────────────── */
    case WM_KILLFOCUS:
        if (kc->m_listening) kc->cancelCapture();
        break;

    /* ── Cleanup ─────────────────────────────────────────────── */
    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, SubclassProc, idSubclass);
        break;
    }

    return DefSubclassProc(hwnd, msg, wp, lp);
}
