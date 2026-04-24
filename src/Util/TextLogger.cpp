// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
#include "TextLogger.h"
#include "../UI/TextArea/TextArea.h"

static std::wstring timestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[32];
    wsprintfW(buf, L"%02u:%02u:%02u.%03u", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}

std::wstring TextLogger::hex(const std::vector<uint8_t>& bytes) {
    std::wstring s;
    s.reserve(bytes.size() * 3);
    wchar_t buf[8];
    for (size_t i = 0; i < bytes.size(); ++i) {
        wsprintfW(buf, L"%02X", bytes[i]);
        s += buf;
        if (i + 1 < bytes.size()) s += L' ';
    }
    return s;
}

void TextLogger::appendLine(const std::wstring& prefix, const std::wstring& msg) {
    std::wstring line = L"[" + timestamp() + L"] " + prefix + L" " + msg + L"\r\n";
    m_buffer += line;
    if (m_sink) m_sink->append(line);
}

void TextLogger::info (const std::wstring& msg) { if (showInfo)  appendLine(L"INFO", msg); }
void TextLogger::error(const std::wstring& msg) { if (showError) appendLine(L"ERR ", msg); }

void TextLogger::tx(const wchar_t* tag, const std::vector<uint8_t>& bytes) {
    if (!showTx) return;
    std::wstring t = tag ? tag : L"";
    appendLine(L"TX  ", t + L"  " + hex(bytes));
}

void TextLogger::rx(const wchar_t* tag, const std::vector<uint8_t>& bytes) {
    if (!showRx) return;
    std::wstring t = tag ? tag : L"";
    appendLine(L"RX  ", t + L"  " + hex(bytes));
}

void TextLogger::clear() {
    m_buffer.clear();
    if (m_sink) m_sink->clear();
}
