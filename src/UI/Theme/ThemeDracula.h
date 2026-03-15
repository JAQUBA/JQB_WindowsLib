// ============================================================================
// ThemeDracula.h — Dracula dark theme color palette
// Part of JQB_WindowsLib
// https://draculatheme.com/
// ============================================================================
#ifndef JQB_THEME_DRACULA_H
#define JQB_THEME_DRACULA_H

#include <windows.h>

// ============================================================================
// Background & surfaces
// ============================================================================
#define CLR_BG          RGB(40, 42, 54)       // Background
#define CLR_BASE        RGB(40, 42, 54)       // Background
#define CLR_SURFACE     RGB(68, 71, 90)       // Current Line
#define CLR_SURFACE2    RGB(68, 71, 90)       // Current Line
#define CLR_SURFACE3    RGB(98, 114, 164)     // Comment

// ============================================================================
// Text
// ============================================================================
#define CLR_TEXT        RGB(248, 248, 242)    // Foreground
#define CLR_SUBTEXT    RGB(189, 147, 249)     // Purple (soft)
#define CLR_TEXT_DIM   RGB(98, 114, 164)      // Comment

// ============================================================================
// Accent colors (Dracula palette)
// ============================================================================
#define CLR_ACCENT     RGB(139, 233, 253)     // Cyan
#define CLR_ACCENT_H   RGB(189, 147, 249)     // Purple
#define CLR_GREEN      RGB(80, 250, 123)      // Green
#define CLR_RED        RGB(255, 85, 85)       // Red
#define CLR_PEACH      RGB(255, 184, 108)     // Orange
#define CLR_MAUVE      RGB(189, 147, 249)     // Purple
#define CLR_YELLOW     RGB(241, 250, 140)     // Yellow
#define CLR_TEAL       RGB(139, 233, 253)     // Cyan (alias)
#define CLR_PINK       RGB(255, 121, 198)     // Pink
#define CLR_FLAMINGO   RGB(255, 184, 108)     // Orange (alias)
#define CLR_LAVENDER   RGB(189, 147, 249)     // Purple (alias)
#define CLR_SAPPHIRE   RGB(139, 233, 253)     // Cyan (alias)
#define CLR_MAROON     RGB(255, 85, 85)       // Red (alias)
#define CLR_ROSEWATER  RGB(255, 121, 198)     // Pink (alias)

// ============================================================================
// UI chrome — buttons
// ============================================================================
#define CLR_SEPARATOR  RGB(68, 71, 90)        // Current Line
#define CLR_BTN_BG     RGB(68, 71, 90)        // Current Line
#define CLR_BTN_HOVER  RGB(98, 114, 164)      // Comment
#define CLR_BTN_ACTIVE CLR_ACCENT

// Secondary / dim buttons
#define CLR_DIM_BG     RGB(55, 57, 72)
#define CLR_DIM_HOVER  RGB(68, 71, 90)
#define CLR_DIM_TEXT   RGB(180, 180, 200)

// ============================================================================
// Semantic — status labels
// ============================================================================
#define CLR_STATUS     RGB(180, 180, 200)
#define CLR_SECTION    RGB(139, 233, 253)     // Cyan

// ============================================================================
// Chart defaults
// ============================================================================
#define CLR_CHART_BG   RGB(55, 57, 72)
#define CLR_CHART_GRID RGB(68, 71, 90)
#define CLR_CHART_AXIS RGB(180, 180, 200)

// ============================================================================
// ProgressBar
// ============================================================================
#define CLR_PROGRESS_BG RGB(55, 57, 72)

#endif /* JQB_THEME_DRACULA_H */
