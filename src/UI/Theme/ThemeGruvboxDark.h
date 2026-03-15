// ============================================================================
// ThemeGruvboxDark.h — Gruvbox Dark theme color palette
// Part of JQB_WindowsLib
// https://github.com/morhetz/gruvbox
// ============================================================================
#ifndef JQB_THEME_GRUVBOX_DARK_H
#define JQB_THEME_GRUVBOX_DARK_H

#include <windows.h>

// ============================================================================
// Background & surfaces
// ============================================================================
#define CLR_BG          RGB(29, 32, 33)       // bg0_h (hard)
#define CLR_BASE        RGB(40, 40, 40)       // bg0
#define CLR_SURFACE     RGB(50, 48, 47)       // bg1
#define CLR_SURFACE2    RGB(60, 56, 54)       // bg2
#define CLR_SURFACE3    RGB(80, 73, 69)       // bg3

// ============================================================================
// Text
// ============================================================================
#define CLR_TEXT        RGB(235, 219, 178)    // fg (light)
#define CLR_SUBTEXT    RGB(213, 196, 161)     // fg2
#define CLR_TEXT_DIM   RGB(168, 153, 132)     // fg4

// ============================================================================
// Accent colors (Gruvbox palette — bright)
// ============================================================================
#define CLR_ACCENT     RGB(131, 165, 152)     // Aqua (bright)
#define CLR_ACCENT_H   RGB(142, 192, 124)     // Green (bright)
#define CLR_GREEN      RGB(184, 187, 38)      // Yellow-Green (bright)
#define CLR_RED        RGB(251, 73, 52)       // Red (bright)
#define CLR_PEACH      RGB(254, 128, 25)      // Orange (bright)
#define CLR_MAUVE      RGB(211, 134, 155)     // Purple (bright)
#define CLR_YELLOW     RGB(250, 189, 47)      // Yellow (bright)
#define CLR_TEAL       RGB(131, 165, 152)     // Aqua (bright)
#define CLR_PINK       RGB(211, 134, 155)     // Purple (alias)
#define CLR_FLAMINGO   RGB(254, 128, 25)      // Orange (alias)
#define CLR_LAVENDER   RGB(131, 165, 152)     // Aqua (alias)
#define CLR_SAPPHIRE   RGB(69, 133, 136)      // Aqua (neutral)
#define CLR_MAROON     RGB(204, 36, 29)       // Red (neutral)
#define CLR_ROSEWATER  RGB(214, 93, 14)       // Orange (neutral)

// ============================================================================
// UI chrome — buttons
// ============================================================================
#define CLR_SEPARATOR  RGB(60, 56, 54)        // bg2
#define CLR_BTN_BG     RGB(60, 56, 54)        // bg2
#define CLR_BTN_HOVER  RGB(80, 73, 69)        // bg3
#define CLR_BTN_ACTIVE CLR_ACCENT

// Secondary / dim buttons
#define CLR_DIM_BG     RGB(50, 48, 47)        // bg1
#define CLR_DIM_HOVER  RGB(60, 56, 54)        // bg2
#define CLR_DIM_TEXT   RGB(189, 174, 147)     // fg3

// ============================================================================
// Semantic — status labels
// ============================================================================
#define CLR_STATUS     RGB(189, 174, 147)     // fg3
#define CLR_SECTION    RGB(131, 165, 152)     // Aqua

// ============================================================================
// Chart defaults
// ============================================================================
#define CLR_CHART_BG   RGB(50, 48, 47)        // bg1
#define CLR_CHART_GRID RGB(80, 73, 69)        // bg3
#define CLR_CHART_AXIS RGB(168, 153, 132)     // fg4

// ============================================================================
// ProgressBar
// ============================================================================
#define CLR_PROGRESS_BG RGB(50, 48, 47)       // bg1

#endif /* JQB_THEME_GRUVBOX_DARK_H */
