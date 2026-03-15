// ============================================================================
// ThemeTokyoNight.h — Tokyo Night dark theme color palette
// Part of JQB_WindowsLib
// https://github.com/enkia/tokyo-night-vscode-theme
// ============================================================================
#ifndef JQB_THEME_TOKYO_NIGHT_H
#define JQB_THEME_TOKYO_NIGHT_H

#include <windows.h>

// ============================================================================
// Background & surfaces
// ============================================================================
#define CLR_BG          RGB(26, 27, 38)       // Editor background
#define CLR_BASE        RGB(36, 40, 59)       // Base
#define CLR_SURFACE     RGB(31, 35, 53)       // Sidebar
#define CLR_SURFACE2    RGB(41, 46, 66)       // Selection
#define CLR_SURFACE3    RGB(55, 60, 82)       // Line highlight

// ============================================================================
// Text
// ============================================================================
#define CLR_TEXT        RGB(169, 177, 214)    // Foreground
#define CLR_SUBTEXT    RGB(130, 140, 180)     // Light comment
#define CLR_TEXT_DIM   RGB(86, 95, 137)       // Comment

// ============================================================================
// Accent colors (Tokyo Night palette)
// ============================================================================
#define CLR_ACCENT     RGB(122, 162, 247)     // Blue
#define CLR_ACCENT_H   RGB(125, 207, 255)     // Cyan
#define CLR_GREEN      RGB(158, 206, 106)     // Green
#define CLR_RED        RGB(247, 118, 142)     // Red
#define CLR_PEACH      RGB(255, 158, 100)     // Orange
#define CLR_MAUVE      RGB(187, 154, 247)     // Magenta
#define CLR_YELLOW     RGB(224, 175, 104)     // Yellow
#define CLR_TEAL       RGB(115, 218, 202)     // Teal
#define CLR_PINK       RGB(255, 117, 166)     // Pink
#define CLR_FLAMINGO   RGB(255, 158, 100)     // Orange (alias)
#define CLR_LAVENDER   RGB(187, 154, 247)     // Magenta (alias)
#define CLR_SAPPHIRE   RGB(125, 207, 255)     // Cyan (alias)
#define CLR_MAROON     RGB(219, 75, 75)       // Dark Red
#define CLR_ROSEWATER  RGB(199, 146, 234)     // Soft Purple

// ============================================================================
// UI chrome — buttons
// ============================================================================
#define CLR_SEPARATOR  RGB(41, 46, 66)        // Selection
#define CLR_BTN_BG     RGB(41, 46, 66)        // Selection
#define CLR_BTN_HOVER  RGB(55, 60, 82)        // Highlight
#define CLR_BTN_ACTIVE CLR_ACCENT

// Secondary / dim buttons
#define CLR_DIM_BG     RGB(34, 38, 55)
#define CLR_DIM_HOVER  RGB(45, 50, 70)
#define CLR_DIM_TEXT   RGB(130, 140, 180)

// ============================================================================
// Semantic — status labels
// ============================================================================
#define CLR_STATUS     RGB(130, 140, 180)
#define CLR_SECTION    RGB(122, 162, 247)     // Blue

// ============================================================================
// Chart defaults
// ============================================================================
#define CLR_CHART_BG   RGB(34, 38, 55)
#define CLR_CHART_GRID RGB(45, 50, 70)
#define CLR_CHART_AXIS RGB(130, 140, 180)

// ============================================================================
// ProgressBar
// ============================================================================
#define CLR_PROGRESS_BG RGB(34, 38, 55)

#endif /* JQB_THEME_TOKYO_NIGHT_H */
