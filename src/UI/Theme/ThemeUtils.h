// ============================================================================
// ThemeUtils.h — Common theme helper functions
// Part of JQB_WindowsLib
// ============================================================================
#ifndef JQB_THEME_UTILS_H
#define JQB_THEME_UTILS_H

#include <windows.h>

// ============================================================================
// disableVisualStyles — Disable visual styles on a control (CheckBox, Select)
//
// Common Controls v6 visual styles ignore SetTextColor() for BS_AUTOCHECKBOX
// and CBS_DROPDOWNLIST. This forces classic rendering that respects
// WM_CTLCOLORBTN / WM_CTLCOLORLISTBOX colors.
// ============================================================================
inline void disableVisualStyles(HWND hwnd) {
    static HMODULE hUxTheme = LoadLibraryW(L"uxtheme.dll");
    typedef HRESULT (WINAPI *SetWindowThemeFunc)(HWND, LPCWSTR, LPCWSTR);
    static SetWindowThemeFunc pSetWindowTheme = hUxTheme
        ? (SetWindowThemeFunc)GetProcAddress(hUxTheme, "SetWindowTheme")
        : nullptr;
    if (pSetWindowTheme)
        pSetWindowTheme(hwnd, L"", L"");
}

#endif /* JQB_THEME_UTILS_H */
