// ============================================================================
// ThemeOneDark.h — One Dark theme color palette (Atom-inspired)
// Part of JQB_WindowsLib
// ============================================================================
#ifndef JQB_THEME_ONE_DARK_H
#define JQB_THEME_ONE_DARK_H

#include <windows.h>

// ============================================================================
// Background & surfaces
// ============================================================================
#define CLR_BG          RGB(33, 37, 43)       // Editor background
#define CLR_BASE        RGB(40, 44, 52)       // Base
#define CLR_SURFACE     RGB(36, 40, 47)       // Gutter
#define CLR_SURFACE2    RGB(50, 56, 66)       // Selection
#define CLR_SURFACE3    RGB(62, 68, 81)       // Line highlight

// ============================================================================
// Text
// ============================================================================
#define CLR_TEXT        RGB(171, 178, 191)    // Foreground
#define CLR_SUBTEXT    RGB(152, 159, 172)     // Light comment
#define CLR_TEXT_DIM   RGB(92, 99, 112)       // Comment

// ============================================================================
// Accent colors (One Dark palette)
// ============================================================================
#define CLR_ACCENT     RGB(97, 175, 239)      // Blue
#define CLR_ACCENT_H   RGB(86, 182, 194)      // Cyan
#define CLR_GREEN      RGB(152, 195, 121)     // Green
#define CLR_RED        RGB(224, 108, 117)     // Red
#define CLR_PEACH      RGB(209, 154, 102)     // Orange / Dark Yellow
#define CLR_MAUVE      RGB(198, 120, 221)     // Magenta
#define CLR_YELLOW     RGB(229, 192, 123)     // Yellow
#define CLR_TEAL       RGB(86, 182, 194)      // Cyan
#define CLR_PINK       RGB(198, 120, 221)     // Magenta (alias)
#define CLR_FLAMINGO   RGB(209, 154, 102)     // Orange (alias)
#define CLR_LAVENDER   RGB(97, 175, 239)      // Blue (alias)
#define CLR_SAPPHIRE   RGB(86, 182, 194)      // Cyan (alias)
#define CLR_MAROON     RGB(224, 108, 117)     // Red (alias)
#define CLR_ROSEWATER  RGB(229, 192, 123)     // Yellow (alias)

// ============================================================================
// UI chrome — buttons
// ============================================================================
#define CLR_SEPARATOR  RGB(50, 56, 66)        // Selection
#define CLR_BTN_BG     RGB(50, 56, 66)        // Selection
#define CLR_BTN_HOVER  RGB(62, 68, 81)        // Line highlight
#define CLR_BTN_ACTIVE CLR_ACCENT

// Secondary / dim buttons
#define CLR_DIM_BG     RGB(38, 42, 50)
#define CLR_DIM_HOVER  RGB(50, 56, 66)
#define CLR_DIM_TEXT   RGB(142, 149, 162)

// ============================================================================
// Semantic — status labels
// ============================================================================
#define CLR_STATUS     RGB(152, 159, 172)
#define CLR_SECTION    RGB(97, 175, 239)      // Blue

// ============================================================================
// Chart defaults
// ============================================================================
#define CLR_CHART_BG   RGB(38, 42, 50)
#define CLR_CHART_GRID RGB(50, 56, 66)
#define CLR_CHART_AXIS RGB(142, 149, 162)

// ============================================================================
// ProgressBar
// ============================================================================
#define CLR_PROGRESS_BG RGB(38, 42, 50)

#endif /* JQB_THEME_ONE_DARK_H */
