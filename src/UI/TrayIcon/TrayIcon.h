// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// TrayIcon — System tray (notification area) icon management
// ============================================================================
#ifndef JQB_TRAY_ICON_H
#define JQB_TRAY_ICON_H

#include <windows.h>
#include <shellapi.h>
#include <functional>
#include <string>

#define WM_TRAYICON (WM_APP + 100)

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();

    // Create the tray icon data (does not show yet — call show())
    bool create(HWND ownerHwnd, UINT iconResourceId, const wchar_t* tooltip);
    void remove();

    // Show / hide the icon in system tray
    void show();
    void hide();
    bool isVisible() const;

    // Callbacks
    void onRestore(std::function<void()> callback);
    void onExit(std::function<void()> callback);

    // Context menu labels (default: "Show" / "Exit")
    void setMenuLabels(const wchar_t* showLabel, const wchar_t* exitLabel);

    // Process window messages (call from subclass proc or WndProc)
    bool processMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    // Menu command IDs
    static const UINT CMD_SHOW = 9200;
    static const UINT CMD_EXIT = 9201;

private:
    HWND m_hwnd = NULL;
    NOTIFYICONDATAW m_nid = {};
    bool m_visible = false;
    std::function<void()> m_onRestore;
    std::function<void()> m_onExit;
    std::wstring m_showLabel = L"Show";
    std::wstring m_exitLabel = L"Exit";

    void showContextMenu();
};

#endif // JQB_TRAY_ICON_H
