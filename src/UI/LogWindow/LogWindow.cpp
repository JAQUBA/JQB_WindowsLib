// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// LogWindow — Standalone log window implementation
// ============================================================================
#include "LogWindow.h"
#include "../../Util/ConfigManager.h"
#include <cstdlib>

const wchar_t* LogWindow::CLASS_NAME = L"JQB_LogWindow";
bool LogWindow::s_classRegistered = false;

LogWindow::LogWindow() {}

LogWindow::~LogWindow() {
    close();
    if (m_font) {
        DeleteObject(m_font);
        m_font = NULL;
    }
    if (m_bgBrush) {
        DeleteObject(m_bgBrush);
        m_bgBrush = NULL;
    }
}

void LogWindow::setTitle(const wchar_t* title) {
    m_title = title;
}

void LogWindow::setTextColor(COLORREF color) {
    m_textColor = color;
}

void LogWindow::setBackColor(COLORREF color) {
    m_backColor = color;
}

void LogWindow::setFont(const wchar_t* fontName, int size) {
    m_fontName = fontName;
    m_fontSize = size;
}

bool LogWindow::open(HWND parentHwnd) {
    if (m_isOpen) {
        SetForegroundWindow(m_hwnd);
        return true;
    }

    if (!s_classRegistered) {
        WNDCLASSW wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandleW(NULL);
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.hCursor = LoadCursorW(NULL, MAKEINTRESOURCEW(32512));
        wc.hIcon = LoadIconW(wc.hInstance, MAKEINTRESOURCEW(101));
        RegisterClassW(&wc);
        s_classRegistered = true;
    }

    int x = 100, y = 100, w = 700, h = 400;
    loadPosition(x, y, w, h);

    m_hwnd = CreateWindowExW(
        0, CLASS_NAME, m_title.c_str(),
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        parentHwnd, NULL, GetModuleHandleW(NULL), this);

    if (!m_hwnd) return false;

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
    m_isOpen = true;
    return true;
}

void LogWindow::close() {
    if (m_isOpen && m_hwnd) {
        savePosition();
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
        m_hEdit = NULL;
        m_isOpen = false;
    }
}

bool LogWindow::isOpen() const {
    return m_isOpen;
}

HWND LogWindow::getHandle() const {
    return m_hwnd;
}

void LogWindow::appendMessage(const wchar_t* msg) {
    if (!m_isOpen || !m_hEdit) return;

    int len = GetWindowTextLengthW(m_hEdit);
    SendMessageW(m_hEdit, EM_SETSEL, len, len);
    SendMessageW(m_hEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);

    int newLen = GetWindowTextLengthW(m_hEdit);
    SendMessageW(m_hEdit, EM_SETSEL, newLen, newLen);
    SendMessageW(m_hEdit, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");

    // Auto-scroll
    SendMessageW(m_hEdit, EM_SCROLLCARET, 0, 0);
}

void LogWindow::clear() {
    if (m_hEdit) {
        SetWindowTextW(m_hEdit, L"");
    }
}

void LogWindow::enablePersistence(ConfigManager& config, const std::string& prefix) {
    m_config = &config;
    m_prefix = prefix;
}

LRESULT CALLBACK LogWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    LogWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<LogWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<LogWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
        case WM_CREATE:
            self->onCreate(hwnd);
            return 0;

        case WM_SIZE:
            self->onSize(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_CLOSE:
            self->handleClose();
            return 0;

        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, self->m_textColor);
            SetBkColor(hdc, self->m_backColor);
            if (!self->m_bgBrush)
                self->m_bgBrush = CreateSolidBrush(self->m_backColor);
            return (LRESULT)self->m_bgBrush;
        }
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void LogWindow::onCreate(HWND hwnd) {
    m_font = CreateFontW(
        -m_fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, m_fontName.c_str());

    RECT rc;
    GetClientRect(hwnd, &rc);

    m_hEdit = CreateWindowExW(
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
        0, 0, rc.right, rc.bottom,
        hwnd, NULL, GetModuleHandleW(NULL), NULL);

    if (m_hEdit && m_font) {
        SendMessageW(m_hEdit, WM_SETFONT, (WPARAM)m_font, TRUE);
    }
}

void LogWindow::onSize(int width, int height) {
    if (m_hEdit) {
        MoveWindow(m_hEdit, 0, 0, width, height, TRUE);
    }
}

void LogWindow::handleClose() {
    savePosition();
    m_isOpen = false;
    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    m_hEdit = NULL;
}

void LogWindow::savePosition() {
    if (!m_config || !m_hwnd) return;
    RECT rc;
    GetWindowRect(m_hwnd, &rc);
    m_config->setValue(m_prefix + "_x", std::to_string(rc.left));
    m_config->setValue(m_prefix + "_y", std::to_string(rc.top));
    m_config->setValue(m_prefix + "_w", std::to_string(rc.right - rc.left));
    m_config->setValue(m_prefix + "_h", std::to_string(rc.bottom - rc.top));
}

void LogWindow::loadPosition(int& x, int& y, int& w, int& h) {
    if (!m_config) return;
    x = atoi(m_config->getValue(m_prefix + "_x", std::to_string(x)).c_str());
    y = atoi(m_config->getValue(m_prefix + "_y", std::to_string(y)).c_str());
    w = atoi(m_config->getValue(m_prefix + "_w", std::to_string(w)).c_str());
    h = atoi(m_config->getValue(m_prefix + "_h", std::to_string(h)).c_str());
}
