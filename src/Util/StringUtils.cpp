#include "StringUtils.h"
#include <vector>
#include <cstring>

namespace StringUtils {
    // Konwersja z UTF-8 na Unicode (UTF-16)
    std::wstring utf8ToWide(const std::string& utf8) {
        if (utf8.empty()) {
            return std::wstring();
        }
        
        // Oblicz wymagany rozmiar bufora
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), NULL, 0);
        if (size_needed <= 0) {
            return std::wstring();
        }
        
        std::wstring wstrTo(size_needed, 0);
        
        // Wykonaj konwersję
        MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &wstrTo[0], size_needed);
        
        return wstrTo;
    }
    
    // Konwersja z Unicode (UTF-16) na UTF-8
    std::string wideToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) {
            return std::string();
        }
        
        // Oblicz wymagany rozmiar bufora
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
        if (size_needed <= 0) {
            return std::string();
        }
        
        std::string strTo(size_needed, 0);
        
        // Wykonaj konwersję
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        
        return strTo;
    }
    
    // Konwersja z wstring na string
    std::string wstringToString(const std::wstring& wstr) {
        // W najprostszej implementacji możemy wykorzystać istniejącą funkcję wideToUtf8
        return wideToUtf8(wstr);
    }
    
    // Wyodrębnia nazwę portu COM z tekstu (np. "USB Serial Port (COM3)" -> "COM3")
    std::string extractComPort(const char* text) {
        if (!text) return "";
        
        // Szukaj "COM" w tekście
        const char* comPos = strstr(text, "COM");
        if (!comPos) return "";
        
        std::string comPort;
        int i = 0;
        
        // Kopiuj znaki od "COM" aż do pierwszego napotkania spacji lub nawiasu zamykającego
        while (comPos[i] && comPos[i] != ' ' && comPos[i] != ')' && i < 10) {
            comPort += comPos[i];
            i++;
        }
        
        return comPort;
    }
}