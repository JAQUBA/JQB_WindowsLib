#include "StringUtils.h"
#include <vector>
#include <cstring>
#include <locale>
#include <codecvt>

namespace StringUtils {
    // Konwersja z UTF-8 na Unicode (UTF-16)
    std::wstring utf8ToWide(const std::string& utf8) {
        if (utf8.empty()) {
            return std::wstring();
        }
        
        try {
            // Upewnij się, że kodowanie wejściowe to UTF-8
            // Użyj funkcji Windows API, która jest bardziej niezawodna niż std::codecvt
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
            if (size_needed <= 0) {
                return std::wstring();
            }
            
            // Alokuj bufor odpowiedniej wielkości (wliczając NULL na końcu)
            std::vector<wchar_t> buffer(size_needed);
            
            // Wykonaj konwersję
            MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &buffer[0], size_needed);
            
            return std::wstring(buffer.data());
        } catch (...) {
            return std::wstring();
        }
    }
    
    // Konwersja z Unicode (UTF-16) na UTF-8
    std::string wideToUtf8(const std::wstring& wstr) {
        if (wstr.empty()) {
            return std::string();
        }
        
        try {
            // Oblicz wymagany rozmiar bufora
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
            if (size_needed <= 0) {
                return std::string();
            }
            
            // Alokuj bufor odpowiedniej wielkości
            std::string strTo(size_needed, 0);
            
            // Wykonaj konwersję
            WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
            
            return strTo;
        } catch (...) {
            return std::string();
        }
    }
    
    // Konwersja z wstring na string
    std::string wstringToString(const std::wstring& wstr) {
        // Użyj konwersji do UTF-8
        return wideToUtf8(wstr);
    }
    
    // Konwersja z ANSI na Unicode (UTF-16)
    std::wstring ansiToWide(const std::string& ansi) {
        if (ansi.empty()) {
            return std::wstring();
        }
        
        try {
            // Oblicz wymagany rozmiar bufora
            int size_needed = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
            if (size_needed <= 0) {
                return std::wstring();
            }
            
            // Alokuj bufor odpowiedniej wielkości
            std::vector<wchar_t> buffer(size_needed);
            
            // Wykonaj konwersję z kodowania ANSI na Unicode
            MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, &buffer[0], size_needed);
            
            return std::wstring(buffer.data());
        } catch (...) {
            return std::wstring();
        }
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