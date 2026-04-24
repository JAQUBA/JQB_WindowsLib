// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
//
// Theme — runtime color palette for SimpleWindow + components.
//
// The library historically shipped Theme*.h files that #define color macros
// (e.g. CLR_BG, CLR_TEXT, CLR_ACCENT). This struct gives you a runtime
// alternative: build a Theme with a few well-known palettes and apply it
// to your window with applyTheme(). The macro headers remain available
// (no deprecation) — both can coexist.
#ifndef JQB_THEME_H
#define JQB_THEME_H

#include "Core.h"

class SimpleWindow;
class Button;
class Label;
class TextArea;
class ProgressBar;

struct Theme {
    // Surfaces
    COLORREF bg          = RGB( 30,  30,  46);  // window background
    COLORREF surface     = RGB( 49,  50,  68);  // raised area
    COLORREF surface2    = RGB( 69,  71,  90);  // hover surface
    // Text
    COLORREF text        = RGB(205, 214, 244);
    COLORREF textDim     = RGB(166, 173, 200);
    // Accents
    COLORREF accent      = RGB(137, 180, 250);  // primary buttons
    COLORREF accentHover = RGB(180, 200, 255);
    // Status
    COLORREF ok          = RGB(166, 227, 161);
    COLORREF warn        = RGB(249, 226, 175);
    COLORREF err         = RGB(243, 139, 168);
    // Default font
    const wchar_t* fontName = L"Segoe UI";
    int fontSize = 10;

    // ---- Built-in palettes ----
    static Theme catppuccinMocha();    // dark, default
    static Theme catppuccinFrappe();   // medium dark
    static Theme catppuccinLatte();    // light
    static Theme nord();               // muted dark blue
    static Theme dracula();            // purple dark
    static Theme tokyoNight();         // deep navy
    static Theme oneDark();            // VS Code "One Dark"
    static Theme gruvboxDark();        // warm dark
    static Theme light();              // simple Windows-light
};

// Apply a theme to a SimpleWindow.
// - sets the window background and the default text color
// - re-paints any Buttons that don't already have explicit colors so they
//   get accent/hover styling consistent with the theme
// - re-paints any ProgressBars that don't already have explicit colors
// - leaves Labels with explicit setBackColor/setTextColor untouched
// Components added AFTER applyTheme() can use Theme.* fields directly.
void applyTheme(SimpleWindow* window, const Theme& th);

// Style a single Button as a primary (accent) button using theme colors.
void styleAccentButton(Button* btn, const Theme& th);

// Style a single Button as a secondary/dim button.
void styleSecondaryButton(Button* btn, const Theme& th);

#endif // JQB_THEME_H
