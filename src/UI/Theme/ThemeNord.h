// ============================================================================
// ThemeNord.h — Nord dark theme color palette
// Part of JQB_WindowsLib
// https://www.nordtheme.com/
// ============================================================================
#ifndef JQB_THEME_NORD_H
#define JQB_THEME_NORD_H

#include <windows.h>

// ============================================================================
// Background & surfaces (Polar Night)
// ============================================================================
#define CLR_BG          RGB(46, 52, 64)       // Nord0
#define CLR_BASE        RGB(46, 52, 64)       // Nord0
#define CLR_SURFACE     RGB(59, 66, 82)       // Nord1
#define CLR_SURFACE2    RGB(67, 76, 94)       // Nord2
#define CLR_SURFACE3    RGB(76, 86, 106)      // Nord3

// ============================================================================
// Text (Snow Storm)
// ============================================================================
#define CLR_TEXT        RGB(236, 239, 244)    // Nord6
#define CLR_SUBTEXT    RGB(229, 233, 240)     // Nord5
#define CLR_TEXT_DIM   RGB(216, 222, 233)     // Nord4

// ============================================================================
// Accent colors (Frost + Aurora)
// ============================================================================
#define CLR_ACCENT     RGB(136, 192, 208)     // Nord8  — Frost (primary)
#define CLR_ACCENT_H   RGB(129, 161, 193)     // Nord9  — Frost
#define CLR_GREEN      RGB(163, 190, 140)     // Nord14 — Aurora Green
#define CLR_RED        RGB(191, 97, 106)      // Nord11 — Aurora Red
#define CLR_PEACH      RGB(208, 135, 112)     // Nord12 — Aurora Orange
#define CLR_MAUVE      RGB(180, 142, 173)     // Nord15 — Aurora Purple
#define CLR_YELLOW     RGB(235, 203, 139)     // Nord13 — Aurora Yellow
#define CLR_TEAL       RGB(143, 188, 187)     // Nord7  — Frost (teal)
#define CLR_PINK       RGB(180, 142, 173)     // Nord15 — Aurora Purple (alias)
#define CLR_FLAMINGO   RGB(208, 135, 112)     // Nord12 — Aurora Orange (alias)
#define CLR_LAVENDER   RGB(129, 161, 193)     // Nord9  — Frost
#define CLR_SAPPHIRE   RGB(94, 129, 172)      // Nord10 — Frost (deep)
#define CLR_MAROON     RGB(191, 97, 106)      // Nord11 — Aurora Red (alias)
#define CLR_ROSEWATER  RGB(235, 203, 139)     // Nord13 — Aurora Yellow (alias)

// ============================================================================
// UI chrome — buttons
// ============================================================================
#define CLR_SEPARATOR  RGB(67, 76, 94)        // Nord2
#define CLR_BTN_BG     RGB(67, 76, 94)        // Nord2
#define CLR_BTN_HOVER  RGB(76, 86, 106)       // Nord3
#define CLR_BTN_ACTIVE CLR_ACCENT

// Secondary / dim buttons
#define CLR_DIM_BG     RGB(59, 66, 82)        // Nord1
#define CLR_DIM_HOVER  RGB(67, 76, 94)        // Nord2
#define CLR_DIM_TEXT   RGB(216, 222, 233)      // Nord4

// ============================================================================
// Semantic — status labels
// ============================================================================
#define CLR_STATUS     RGB(216, 222, 233)     // Nord4
#define CLR_SECTION    RGB(136, 192, 208)     // Nord8

// ============================================================================
// Chart defaults
// ============================================================================
#define CLR_CHART_BG   RGB(59, 66, 82)        // Nord1
#define CLR_CHART_GRID RGB(76, 86, 106)       // Nord3
#define CLR_CHART_AXIS RGB(216, 222, 233)     // Nord4

// ============================================================================
// ProgressBar
// ============================================================================
#define CLR_PROGRESS_BG RGB(59, 66, 82)       // Nord1

#endif /* JQB_THEME_NORD_H */
