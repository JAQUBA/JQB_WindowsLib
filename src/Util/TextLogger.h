// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
//
// TextLogger — generic text-area sink for INFO/ERROR/TX/RX log lines.
// Each level can be filtered on/off independently. Lines are timestamped.
// Useful for serial monitors, Modbus consoles, debug viewers etc.
#ifndef JQB_TEXT_LOGGER_H
#define JQB_TEXT_LOGGER_H

#include "Core.h"
#include <string>
#include <vector>
#include <cstdint>

class TextArea;

class TextLogger {
public:
    void attach(TextArea* sink) { m_sink = sink; }

    // Filters
    bool showInfo  = true;
    bool showError = true;
    bool showTx    = true;
    bool showRx    = true;

    // Logging API
    void info  (const std::wstring& msg);
    void error (const std::wstring& msg);
    void tx    (const wchar_t* tag, const std::vector<uint8_t>& bytes);
    void rx    (const wchar_t* tag, const std::vector<uint8_t>& bytes);

    // Buffer access
    std::wstring snapshot() const { return m_buffer; }
    void         clear();

private:
    void appendLine(const std::wstring& prefix, const std::wstring& msg);
    static std::wstring hex(const std::vector<uint8_t>& bytes);

    TextArea*    m_sink = nullptr;
    std::wstring m_buffer;
};

#endif // JQB_TEXT_LOGGER_H
