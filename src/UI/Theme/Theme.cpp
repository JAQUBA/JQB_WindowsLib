// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
#include "Theme.h"
#include "../SimpleWindow/SimpleWindow.h"
#include "../Button/Button.h"
#include "../ProgressBar/ProgressBar.h"

// ============================================================================
// Built-in palettes
// ============================================================================
Theme Theme::catppuccinMocha() {
    Theme t;
    t.bg          = RGB( 30,  30,  46);
    t.surface     = RGB( 49,  50,  68);
    t.surface2    = RGB( 69,  71,  90);
    t.text        = RGB(205, 214, 244);
    t.textDim     = RGB(166, 173, 200);
    t.accent      = RGB(137, 180, 250);
    t.accentHover = RGB(180, 200, 255);
    t.ok          = RGB(166, 227, 161);
    t.warn        = RGB(249, 226, 175);
    t.err         = RGB(243, 139, 168);
    return t;
}
Theme Theme::catppuccinFrappe() {
    Theme t;
    t.bg          = RGB( 48,  52,  70);
    t.surface     = RGB( 65,  69,  89);
    t.surface2    = RGB( 81,  87, 109);
    t.text        = RGB(198, 208, 245);
    t.textDim     = RGB(165, 173, 206);
    t.accent      = RGB(140, 170, 238);
    t.accentHover = RGB(180, 200, 250);
    t.ok          = RGB(166, 209, 137);
    t.warn        = RGB(229, 200, 144);
    t.err         = RGB(231, 130, 132);
    return t;
}
Theme Theme::catppuccinLatte() {
    Theme t;
    t.bg          = RGB(239, 241, 245);
    t.surface     = RGB(220, 224, 232);
    t.surface2    = RGB(204, 208, 218);
    t.text        = RGB( 76,  79, 105);
    t.textDim     = RGB(108, 111, 133);
    t.accent      = RGB( 30, 102, 245);
    t.accentHover = RGB( 70, 130, 245);
    t.ok          = RGB( 64, 160,  43);
    t.warn        = RGB(223, 142,  29);
    t.err         = RGB(210,  15,  57);
    return t;
}
Theme Theme::nord() {
    Theme t;
    t.bg          = RGB( 46,  52,  64);
    t.surface     = RGB( 59,  66,  82);
    t.surface2    = RGB( 76,  86, 106);
    t.text        = RGB(216, 222, 233);
    t.textDim     = RGB(180, 192, 209);
    t.accent      = RGB(136, 192, 208);
    t.accentHover = RGB(143, 188, 187);
    t.ok          = RGB(163, 190, 140);
    t.warn        = RGB(235, 203, 139);
    t.err         = RGB(191,  97, 106);
    return t;
}
Theme Theme::dracula() {
    Theme t;
    t.bg          = RGB( 40,  42,  54);
    t.surface     = RGB( 68,  71,  90);
    t.surface2    = RGB( 98, 114, 164);
    t.text        = RGB(248, 248, 242);
    t.textDim     = RGB(189, 147, 249);
    t.accent      = RGB(189, 147, 249);
    t.accentHover = RGB(255, 121, 198);
    t.ok          = RGB( 80, 250, 123);
    t.warn        = RGB(241, 250, 140);
    t.err         = RGB(255,  85,  85);
    return t;
}
Theme Theme::tokyoNight() {
    Theme t;
    t.bg          = RGB( 26,  27,  38);
    t.surface     = RGB( 36,  40,  59);
    t.surface2    = RGB( 65,  72,  104);
    t.text        = RGB(192, 202, 245);
    t.textDim     = RGB(154, 165, 206);
    t.accent      = RGB(122, 162, 247);
    t.accentHover = RGB(180, 200, 250);
    t.ok          = RGB(158, 206, 106);
    t.warn        = RGB(224, 175, 104);
    t.err         = RGB(247, 118, 142);
    return t;
}
Theme Theme::oneDark() {
    Theme t;
    t.bg          = RGB( 40,  44,  52);
    t.surface     = RGB( 51,  56,  66);
    t.surface2    = RGB( 76,  82,  99);
    t.text        = RGB(171, 178, 191);
    t.textDim     = RGB(130, 137, 151);
    t.accent      = RGB( 97, 175, 239);
    t.accentHover = RGB(150, 200, 245);
    t.ok          = RGB(152, 195, 121);
    t.warn        = RGB(229, 192, 123);
    t.err         = RGB(224, 108, 117);
    return t;
}
Theme Theme::gruvboxDark() {
    Theme t;
    t.bg          = RGB( 40,  40,  40);
    t.surface     = RGB( 60,  56,  54);
    t.surface2    = RGB( 80,  73,  69);
    t.text        = RGB(235, 219, 178);
    t.textDim     = RGB(189, 174, 147);
    t.accent      = RGB(131, 165, 152);
    t.accentHover = RGB(184, 187, 38);
    t.ok          = RGB(184, 187,  38);
    t.warn        = RGB(250, 189,  47);
    t.err         = RGB(251,  73,  52);
    return t;
}
Theme Theme::light() {
    Theme t;
    t.bg          = RGB(245, 246, 250);
    t.surface     = RGB(230, 232, 238);
    t.surface2    = RGB(214, 218, 226);
    t.text        = RGB( 30,  35,  45);
    t.textDim     = RGB(110, 115, 125);
    t.accent      = RGB(  0, 120, 215);
    t.accentHover = RGB( 30, 150, 235);
    t.ok          = RGB( 16, 124,  16);
    t.warn        = RGB(202, 110,  20);
    t.err         = RGB(196,  43,  28);
    return t;
}

// ============================================================================
// Helpers
// ============================================================================
void styleAccentButton(Button* btn, const Theme& th) {
    if (!btn) return;
    btn->setBackColor (th.accent);
    btn->setTextColor (th.bg);
    btn->setHoverColor(th.accentHover);
    btn->setFont(th.fontName, th.fontSize, true);
}

void styleSecondaryButton(Button* btn, const Theme& th) {
    if (!btn) return;
    btn->setBackColor (th.surface);
    btn->setTextColor (th.text);
    btn->setHoverColor(th.surface2);
    btn->setFont(th.fontName, th.fontSize, false);
}

void applyTheme(SimpleWindow* window, const Theme& th) {
    if (!window) return;
    window->setBackgroundColor(th.bg);
    window->setTextColor      (th.text);

    // Auto-style buttons that have no custom colors yet — secondary by default.
    for (auto* btn : window->getButtons()) {
        if (!btn) continue;
        if (!btn->hasCustomColors())
            styleSecondaryButton(btn, th);
    }

    // Re-paint progress bars without explicit colors.
    for (auto* comp : window->getComponents()) {
        if (auto* pb = dynamic_cast<ProgressBar*>(comp)) {
            // ProgressBar is conservative about its color API; just set a
            // theme-aware bar color. Users may override afterwards.
            pb->setColor   (th.accent);
            pb->setBackColor(th.surface);
        }
    }
}
