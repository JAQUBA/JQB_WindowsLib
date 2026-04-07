// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

/**
 * HotkeyManager.h — System skrótów klawiaturowych
 *
 * Obsługuje globalny hook klawiatury (WH_KEYBOARD_LL),
 * tryb przechwytywania (capture) nowych skrótów,
 * konwersję VK ↔ nazwy klawiszy oraz zapis/odczyt z ConfigManager.
 */

#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include <windows.h>
#include <string>
#include <vector>
#include <functional>
#include <set>

class ConfigManager;

/** Definicja jednego skrótu klawiaturowego */
struct HotkeyDef {
    std::string iniKey;
    std::string label;
    std::string defaultBind;
    std::function<void()> action;

    UINT vk        = 0;
    bool needCtrl  = false;
    bool needShift = false;
    bool needAlt   = false;

    HWND captureHwnd = NULL;
};

/**
 * Menedżer skrótów klawiaturowych.
 */
class HotkeyManager {
public:
    HotkeyManager(ConfigManager& config);
    ~HotkeyManager();

    /** Rejestruje nowy skrót. Zwraca indeks. */
    int  addHotkey(const std::string& iniKey, const std::string& label,
                   const std::string& defaultBind, std::function<void()> action);

    /** Wczytuje bindingi z pliku konfiguracyjnego */
    void loadFromConfig();

    /** Instaluje globalny hook klawiatury */
    void installHook();

    /** Deinstaluje hook */
    void uninstallHook();

    /** Rozpoczyna tryb capture dla danego skrótu (indeks). opcjonalnie ustawia HWND wyświetlacza */
    void startCapture(int index, HWND displayHwnd = NULL);

    /** Otwiera modalny dialog do konfiguracji skrótów klawiaturowych */
    void showSettingsDialog(HWND parent);

    /** Przywraca domyślne bindingi */
    void restoreDefaults();

    std::vector<HotkeyDef>&       getHotkeys()       { return m_hotkeys; }
    const std::vector<HotkeyDef>& getHotkeys() const { return m_hotkeys; }
    HotkeyDef&       getHotkey(int i)       { return m_hotkeys[i]; }
    const HotkeyDef& getHotkey(int i) const { return m_hotkeys[i]; }
    size_t count() const { return m_hotkeys.size(); }

    void setOnShortcutsChanged(std::function<void()> cb) { m_onShortcutsChanged = cb; }

    static UINT        nameToVK(const std::string& name);
    static std::string vkToName(UINT vk);
    static bool        parseBinding(const std::string& str, UINT& vk,
                                    bool& ctrl, bool& shift, bool& alt);
    static std::string buildBindString(UINT vk, bool ctrl, bool shift, bool alt);

private:
    std::vector<HotkeyDef> m_hotkeys;
    ConfigManager& m_config;

    bool m_captureMode  = false;
    int  m_captureIndex = -1;

    HHOOK m_keyboardHook = NULL;
    std::set<UINT> m_pressedVKs;
    std::function<void()> m_onShortcutsChanged;

    static HotkeyManager* s_instance;
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    static bool isModifierVK(UINT vk);

    // Dialog skrótów klawiaturowych
    HWND m_settingsDlg = NULL;
    std::vector<HWND> m_dlgBindingButtons;
    static LRESULT CALLBACK SettingsDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif // HOTKEY_MANAGER_H
