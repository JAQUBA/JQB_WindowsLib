// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

/**
 * HotkeyManager.cpp — Implementacja systemu skrótów klawiaturowych
 */

#include "HotkeyManager.h"
#include "ConfigManager.h"
#include "../Util/StringUtils.h"
#include "Core.h"

// Statyczna instancja dla callbacku hooka
HotkeyManager* HotkeyManager::s_instance = nullptr;

// ============================================================================
// Konstruktor / Destruktor
// ============================================================================

HotkeyManager::HotkeyManager(ConfigManager& config)
    : m_config(config)
{
    s_instance = this;
}

HotkeyManager::~HotkeyManager() {
    uninstallHook();
    if (s_instance == this) s_instance = nullptr;
}

// ============================================================================
// Rejestracja skrótów
// ============================================================================

int HotkeyManager::addHotkey(const std::string& iniKey,
                              const std::string& label,
                              const std::string& defaultBind,
                              std::function<void()> action)
{
    HotkeyDef def;
    def.iniKey      = iniKey;
    def.label       = label;
    def.defaultBind = defaultBind;
    def.action      = action;
    parseBinding(defaultBind, def.vk, def.needCtrl, def.needShift, def.needAlt);
    m_hotkeys.push_back(def);
    return static_cast<int>(m_hotkeys.size()) - 1;
}

// ============================================================================
// Wczytaj z konfiguracji
// ============================================================================

void HotkeyManager::loadFromConfig() {
    for (auto& hk : m_hotkeys) {
        std::string val = m_config.getValue(hk.iniKey, hk.defaultBind);
        if (!val.empty()) {
            parseBinding(val, hk.vk, hk.needCtrl, hk.needShift, hk.needAlt);
        }
    }
}

// ============================================================================
// Przywróć domyślne
// ============================================================================

void HotkeyManager::restoreDefaults() {
    for (auto& hk : m_hotkeys) {
        parseBinding(hk.defaultBind, hk.vk, hk.needCtrl, hk.needShift, hk.needAlt);
        m_config.setValue(hk.iniKey, hk.defaultBind);

        if (hk.captureHwnd) {
            std::string bindStr = buildBindString(hk.vk, hk.needCtrl,
                                                   hk.needShift, hk.needAlt);
            SetWindowTextA(hk.captureHwnd, bindStr.c_str());
        }
    }
    if (m_onShortcutsChanged) m_onShortcutsChanged();
}

// ============================================================================
// Hook klawiatury
// ============================================================================

void HotkeyManager::installHook() {
    if (m_keyboardHook) return;
    m_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL,
                                        LowLevelKeyboardProc,
                                        GetModuleHandleW(NULL), 0);
}

void HotkeyManager::uninstallHook() {
    if (m_keyboardHook) {
        UnhookWindowsHookEx(m_keyboardHook);
        m_keyboardHook = NULL;
    }
}

// ============================================================================
// Tryb capture (przechwytywanie nowego skrótu)
// ============================================================================

void HotkeyManager::startCapture(int index, HWND displayHwnd) {
    if (index < 0 || index >= static_cast<int>(m_hotkeys.size())) return;

    // Przywróć tekst poprzedniego przycisku jeśli był w trybie capture
    if (m_captureMode && m_captureIndex >= 0 && m_captureIndex < (int)m_hotkeys.size()) {
        auto& prev = m_hotkeys[m_captureIndex];
        if (prev.captureHwnd) {
            std::string prevBind = buildBindString(prev.vk, prev.needCtrl,
                                                    prev.needShift, prev.needAlt);
            SetWindowTextA(prev.captureHwnd, prevBind.c_str());
        }
    }

    m_captureMode  = true;
    m_captureIndex = index;
    m_pressedVKs.clear();

    if (displayHwnd) m_hotkeys[index].captureHwnd = displayHwnd;
    if (m_hotkeys[index].captureHwnd) {
        SetWindowTextA(m_hotkeys[index].captureHwnd, "...");
    }
}

// ============================================================================
// Low-Level Keyboard Proc
// ============================================================================

LRESULT CALLBACK HotkeyManager::LowLevelKeyboardProc(int nCode, WPARAM wParam,
                                                       LPARAM lParam)
{
    if (nCode == HC_ACTION && s_instance) {
        KBDLLHOOKSTRUCT* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        UINT vk = kb->vkCode;

        // --- Tryb capture ---
        if (s_instance->m_captureMode) {
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                s_instance->m_pressedVKs.insert(vk);
            }
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                // Zakończ capture przy puszczeniu klawisza
                if (!s_instance->m_pressedVKs.empty()) {
                    int idx = s_instance->m_captureIndex;
                    auto& hk = s_instance->m_hotkeys[idx];

                    // Znajdź klawisz główny (nie-modifier)
                    UINT mainVK = 0;
                    bool ctrl = false, shift = false, alt = false;
                    for (UINT k : s_instance->m_pressedVKs) {
                        if (k == VK_LCONTROL || k == VK_RCONTROL || k == VK_CONTROL)
                            ctrl = true;
                        else if (k == VK_LSHIFT || k == VK_RSHIFT || k == VK_SHIFT)
                            shift = true;
                        else if (k == VK_LMENU || k == VK_RMENU || k == VK_MENU)
                            alt = true;
                        else
                            mainVK = k;
                    }

                    if (mainVK != 0) {
                        // Escape bez modyfikatorów = anuluj capture
                        if (mainVK == VK_ESCAPE && !ctrl && !shift && !alt) {
                            if (hk.captureHwnd) {
                                std::string prevBind = buildBindString(
                                    hk.vk, hk.needCtrl, hk.needShift, hk.needAlt);
                                SetWindowTextA(hk.captureHwnd, prevBind.c_str());
                            }
                            s_instance->m_captureMode = false;
                            s_instance->m_captureIndex = -1;
                            s_instance->m_pressedVKs.clear();
                            return 1;
                        }

                        hk.vk        = mainVK;
                        hk.needCtrl  = ctrl;
                        hk.needShift = shift;
                        hk.needAlt   = alt;

                        std::string bindStr = buildBindString(mainVK, ctrl, shift, alt);
                        s_instance->m_config.setValue(hk.iniKey, bindStr);

                        if (hk.captureHwnd) {
                            SetWindowTextA(hk.captureHwnd, bindStr.c_str());
                        }
                        if (s_instance->m_onShortcutsChanged)
                            s_instance->m_onShortcutsChanged();
                    }

                    s_instance->m_captureMode = false;
                    s_instance->m_captureIndex = -1;
                    s_instance->m_pressedVKs.clear();
                }
                return 1; // Pochłoń klawisz
            }
            return 1; // Pochłoń w trybie capture
        }

        // --- Normalny tryb — sprawdź skróty ---
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool shift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;
            bool alt   = (GetAsyncKeyState(VK_MENU)    & 0x8000) != 0;

            for (auto& hk : s_instance->m_hotkeys) {
                if (hk.vk == vk &&
                    hk.needCtrl  == ctrl &&
                    hk.needShift == shift &&
                    hk.needAlt   == alt)
                {
                    if (hk.action) hk.action();
                    return 1;
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

bool HotkeyManager::isModifierVK(UINT vk) {
    return vk == VK_CONTROL || vk == VK_LCONTROL || vk == VK_RCONTROL ||
           vk == VK_SHIFT   || vk == VK_LSHIFT   || vk == VK_RSHIFT   ||
           vk == VK_MENU    || vk == VK_LMENU    || vk == VK_RMENU;
}

// ============================================================================
// Konwersje VK ↔ nazwa
// ============================================================================

UINT HotkeyManager::nameToVK(const std::string& name) {
    if (name == "F1")  return VK_F1;
    if (name == "F2")  return VK_F2;
    if (name == "F3")  return VK_F3;
    if (name == "F4")  return VK_F4;
    if (name == "F5")  return VK_F5;
    if (name == "F6")  return VK_F6;
    if (name == "F7")  return VK_F7;
    if (name == "F8")  return VK_F8;
    if (name == "F9")  return VK_F9;
    if (name == "F10") return VK_F10;
    if (name == "F11") return VK_F11;
    if (name == "F12") return VK_F12;
    if (name == "Space")  return VK_SPACE;
    if (name == "Enter")  return VK_RETURN;
    if (name == "Escape") return VK_ESCAPE;
    if (name == "Tab")    return VK_TAB;
    if (name == "Delete") return VK_DELETE;
    if (name == "Insert") return VK_INSERT;
    if (name == "Home")   return VK_HOME;
    if (name == "End")    return VK_END;
    if (name == "PageUp")   return VK_PRIOR;
    if (name == "PageDown") return VK_NEXT;
    if (name == "Up")    return VK_UP;
    if (name == "Down")  return VK_DOWN;
    if (name == "Left")  return VK_LEFT;
    if (name == "Right") return VK_RIGHT;
    if (name == "Backspace") return VK_BACK;

    // Cyfry 0-9
    if (name.size() == 1 && name[0] >= '0' && name[0] <= '9')
        return static_cast<UINT>(name[0]);
    // Litery A-Z
    if (name.size() == 1 && name[0] >= 'A' && name[0] <= 'Z')
        return static_cast<UINT>(name[0]);
    if (name.size() == 1 && name[0] >= 'a' && name[0] <= 'z')
        return static_cast<UINT>(name[0] - 'a' + 'A');

    return 0;
}

std::string HotkeyManager::vkToName(UINT vk) {
    if (vk >= VK_F1 && vk <= VK_F12) {
        char buf[8];
        snprintf(buf, sizeof(buf), "F%d", vk - VK_F1 + 1);
        return buf;
    }
    switch (vk) {
        case VK_SPACE:  return "Space";
        case VK_RETURN: return "Enter";
        case VK_ESCAPE: return "Escape";
        case VK_TAB:    return "Tab";
        case VK_DELETE: return "Delete";
        case VK_INSERT: return "Insert";
        case VK_HOME:   return "Home";
        case VK_END:    return "End";
        case VK_PRIOR:  return "PageUp";
        case VK_NEXT:   return "PageDown";
        case VK_UP:     return "Up";
        case VK_DOWN:   return "Down";
        case VK_LEFT:   return "Left";
        case VK_RIGHT:  return "Right";
        case VK_BACK:   return "Backspace";
    }
    // Litery i cyfry
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) {
        return std::string(1, static_cast<char>(vk));
    }
    // Nieznany — pokaż numer
    char buf[16];
    snprintf(buf, sizeof(buf), "VK_%u", vk);
    return buf;
}

// ============================================================================
// Parsowanie bindingów (np. "Ctrl+Shift+F5")
// ============================================================================

bool HotkeyManager::parseBinding(const std::string& str, UINT& vk,
                                  bool& ctrl, bool& shift, bool& alt)
{
    vk = 0; ctrl = false; shift = false; alt = false;
    if (str.empty()) return false;

    // Rozdziel po '+'
    std::string part;
    std::string remaining = str;
    while (!remaining.empty()) {
        size_t pos = remaining.find('+');
        if (pos != std::string::npos) {
            part = remaining.substr(0, pos);
            remaining = remaining.substr(pos + 1);
        } else {
            part = remaining;
            remaining.clear();
        }
        // Trim
        while (!part.empty() && part[0] == ' ') part.erase(0, 1);
        while (!part.empty() && part.back() == ' ') part.pop_back();

        if (part == "Ctrl")       ctrl  = true;
        else if (part == "Shift") shift = true;
        else if (part == "Alt")   alt   = true;
        else                      vk    = nameToVK(part);
    }
    return vk != 0;
}

std::string HotkeyManager::buildBindString(UINT vk, bool ctrl,
                                            bool shift, bool alt)
{
    std::string result;
    if (ctrl)  result += "Ctrl+";
    if (shift) result += "Shift+";
    if (alt)   result += "Alt+";
    result += vkToName(vk);
    return result;
}

// ============================================================================
// Dialog skrótów klawiaturowych — WndProc
// ============================================================================

#define IDB_HK_BASE     8100
#define IDB_HK_DEFAULTS 8190
#define IDB_HK_CLOSE    8191

LRESULT CALLBACK HotkeyManager::SettingsDlgProc(HWND hwnd, UINT uMsg,
                                                  WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (!s_instance) break;

            // Kliknięcie przycisku skrótu — rozpocznij przechwytywanie
            if (id >= IDB_HK_BASE &&
                id < IDB_HK_BASE + (int)s_instance->m_hotkeys.size())
            {
                int idx = id - IDB_HK_BASE;
                s_instance->startCapture(idx, s_instance->m_dlgBindingButtons[idx]);
                return 0;
            }

            // Przywróć domyślne
            if (id == IDB_HK_DEFAULTS) {
                s_instance->restoreDefaults();
                for (size_t i = 0; i < s_instance->m_hotkeys.size(); i++) {
                    auto& hk = s_instance->m_hotkeys[i];
                    std::string bindStr = buildBindString(
                        hk.vk, hk.needCtrl, hk.needShift, hk.needAlt);
                    SetWindowTextA(s_instance->m_dlgBindingButtons[i], bindStr.c_str());
                }
                return 0;
            }

            if (id == IDB_HK_CLOSE) {
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY: {
            if (s_instance) {
                s_instance->m_settingsDlg = NULL;
                s_instance->m_dlgBindingButtons.clear();
                for (auto& hk : s_instance->m_hotkeys)
                    hk.captureHwnd = NULL;
            }
            HWND owner = GetWindow(hwnd, GW_OWNER);
            if (owner) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

// ============================================================================
// Dialog skrótów klawiaturowych — otwarcie
// ============================================================================

void HotkeyManager::showSettingsDialog(HWND parent) {
    if (m_settingsDlg) {
        SetForegroundWindow(m_settingsDlg);
        return;
    }

    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc  = SettingsDlgProc;
        wc.hInstance     = _core.hInstance;
        wc.lpszClassName = L"JQB_HotkeySettingsDlg";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
        RegisterClassW(&wc);
        classRegistered = true;
    }

    int count = (int)m_hotkeys.size();
    int dlgW = 420;
    int dlgH = 50 + count * 36 + 55;

    RECT pr;
    GetWindowRect(parent, &pr);
    int x = pr.left + ((pr.right - pr.left) - dlgW) / 2;
    int y = pr.top  + ((pr.bottom - pr.top) - dlgH) / 2;

    RECT rc = { 0, 0, dlgW, dlgH };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE);

    m_settingsDlg = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        L"JQB_HotkeySettingsDlg",
        L"Skróty klawiaturowe",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        x, y,
        rc.right - rc.left, rc.bottom - rc.top,
        parent, NULL, _core.hInstance, NULL);

    if (!m_settingsDlg) return;

    EnableWindow(parent, FALSE);

    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    int cy = 15;
    m_dlgBindingButtons.clear();

    for (int i = 0; i < count; i++) {
        auto& hk = m_hotkeys[i];
        std::wstring label = StringUtils::utf8ToWide(hk.label) + L":";

        HWND hLabel = CreateWindowExW(0, L"STATIC", label.c_str(),
            WS_CHILD | WS_VISIBLE | SS_RIGHT,
            15, cy + 4, 140, 20,
            m_settingsDlg, NULL, _core.hInstance, NULL);
        SendMessageW(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

        std::string bindStr = buildBindString(
            hk.vk, hk.needCtrl, hk.needShift, hk.needAlt);

        HWND hBtn = CreateWindowExW(0, L"BUTTON",
            StringUtils::utf8ToWide(bindStr).c_str(),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            165, cy, 200, 28,
            m_settingsDlg, (HMENU)(INT_PTR)(IDB_HK_BASE + i),
            _core.hInstance, NULL);
        SendMessageW(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

        m_dlgBindingButtons.push_back(hBtn);
        hk.captureHwnd = hBtn;

        cy += 36;
    }

    HWND hDefaults = CreateWindowExW(0, L"BUTTON", L"Przywróć domyślne",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        15, cy + 10, 150, 30,
        m_settingsDlg, (HMENU)(INT_PTR)IDB_HK_DEFAULTS,
        _core.hInstance, NULL);
    SendMessageW(hDefaults, WM_SETFONT, (WPARAM)hFont, TRUE);

    HWND hClose = CreateWindowExW(0, L"BUTTON", L"Zamknij",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        dlgW - 115, cy + 10, 90, 30,
        m_settingsDlg, (HMENU)(INT_PTR)IDB_HK_CLOSE,
        _core.hInstance, NULL);
    SendMessageW(hClose, WM_SETFONT, (WPARAM)hFont, TRUE);

    ShowWindow(m_settingsDlg, SW_SHOW);
    UpdateWindow(m_settingsDlg);
}
