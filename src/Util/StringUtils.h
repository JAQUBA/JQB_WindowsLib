// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>
#include <windows.h>

namespace StringUtils {
    // Konwersja z UTF-8 na Unicode (UTF-16)
    std::wstring utf8ToWide(const std::string& utf8);
    
    // Konwersja z Unicode (UTF-16) na UTF-8
    std::string wideToUtf8(const std::wstring& wstr);
    
    // Konwersja z wstring na string
    std::string wstringToString(const std::wstring& wstr);
    
    // Konwersja z ANSI na Unicode (UTF-16)
    std::wstring ansiToWide(const std::string& ansi);
    
    // Wyodrębnia nazwę portu COM z tekstu (np. "USB Serial Port (COM3)" -> "COM3")
    std::string extractComPort(const char* text);
}

#endif // STRING_UTILS_H