// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// LogWindow — Standalone log/console window with persistent position
// ============================================================================
#ifndef JQB_LOG_WINDOW_H
#define JQB_LOG_WINDOW_H

#include <windows.h>
#include <string>

class ConfigManager;

class LogWindow {
public:
    LogWindow();
    ~LogWindow();

    // Set window title (call before open)
    void setTitle(const wchar_t* title);

    // Set colors (call before open)
    void setTextColor(COLORREF color);
    void setBackColor(COLORREF color);

    // Set font (call before open)
    void setFont(const wchar_t* fontName, int size);

    bool open(HWND parentHwnd = NULL);
    void close();
    bool isOpen() const;
    HWND getHandle() const;

    void appendMessage(const wchar_t* msg);
    void clear();

    // Enable auto-save/load of window position to ConfigManager
    void enablePersistence(ConfigManager& config, const std::string& prefix = "logwin");

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void onCreate(HWND hwnd);
    void onSize(int width, int height);
    void handleClose();

    HWND m_hwnd = NULL;
    HWND m_hEdit = NULL;
    HFONT m_font = NULL;
    HBRUSH m_bgBrush = NULL;
    bool m_isOpen = false;

    // Configurable appearance
    std::wstring m_title    = L"Log";
    std::wstring m_fontName = L"Consolas";
    int          m_fontSize = 14;
    COLORREF     m_textColor = RGB(170, 180, 195);
    COLORREF     m_backColor = RGB(22, 22, 28);

    // Persistence
    ConfigManager* m_config = nullptr;
    std::string m_prefix;
    void savePosition();
    void loadPosition(int& x, int& y, int& w, int& h);

    static const wchar_t* CLASS_NAME;
    static bool s_classRegistered;
};

#endif // JQB_LOG_WINDOW_H
