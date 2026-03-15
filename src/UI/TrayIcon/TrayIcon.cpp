// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// TrayIcon — System tray icon implementation
// ============================================================================
#include "TrayIcon.h"

TrayIcon::TrayIcon() {}

TrayIcon::~TrayIcon() {
    remove();
}

bool TrayIcon::create(HWND ownerHwnd, UINT iconResourceId, const wchar_t* tooltip) {
    m_hwnd = ownerHwnd;

    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = ownerHwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(iconResourceId));

    wcsncpy(m_nid.szTip, tooltip, 127);
    m_nid.szTip[127] = L'\0';

    return (m_nid.hIcon != NULL);
}

void TrayIcon::remove() {
    if (m_visible) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_visible = false;
    }
}

void TrayIcon::show() {
    if (!m_visible) {
        Shell_NotifyIconW(NIM_ADD, &m_nid);
        m_visible = true;
    }
}

void TrayIcon::hide() {
    if (m_visible) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_visible = false;
    }
}

bool TrayIcon::isVisible() const {
    return m_visible;
}

void TrayIcon::onRestore(std::function<void()> callback) {
    m_onRestore = callback;
}

void TrayIcon::onExit(std::function<void()> callback) {
    m_onExit = callback;
}

void TrayIcon::setMenuLabels(const wchar_t* showLabel, const wchar_t* exitLabel) {
    m_showLabel = showLabel;
    m_exitLabel = exitLabel;
}

bool TrayIcon::processMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        switch (LOWORD(lParam)) {
            case WM_LBUTTONDBLCLK:
                if (m_onRestore) m_onRestore();
                return true;
            case WM_RBUTTONUP:
                showContextMenu();
                return true;
        }
        return true;
    }

    if (msg == WM_COMMAND) {
        switch (LOWORD(wParam)) {
            case CMD_SHOW:
                if (m_onRestore) m_onRestore();
                return true;
            case CMD_EXIT:
                if (m_onExit)
                    m_onExit();
                else
                    PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
                return true;
        }
    }

    return false;
}

void TrayIcon::showContextMenu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, CMD_SHOW, m_showLabel.c_str());
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, CMD_EXIT, m_exitLabel.c_str());

    SetForegroundWindow(m_hwnd);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_hwnd, NULL);
    PostMessageW(m_hwnd, WM_NULL, 0, 0);
    DestroyMenu(menu);
}
