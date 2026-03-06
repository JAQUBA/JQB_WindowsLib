/*
 * KeyCapture.h — Owner-draw key-capture button for recording keyboard combos
 *
 * A UIComponent that lets the user click a button and then press a key
 * combination (e.g. Ctrl+Alt+F6). The captured combo is stored as a
 * Windows VK code + modifier bitmask (HID-standard layout).
 */
#ifndef KEY_CAPTURE_H
#define KEY_CAPTURE_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <functional>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Modifier bitmask — standard HID keyboard modifier byte layout     */
/* ------------------------------------------------------------------ */
#define KCMOD_LCTRL   0x01
#define KCMOD_LSHIFT  0x02
#define KCMOD_LALT    0x04
#define KCMOD_LGUI    0x08
#define KCMOD_RCTRL   0x10
#define KCMOD_RSHIFT  0x20
#define KCMOD_RALT    0x40
#define KCMOD_RGUI    0x80

/* ------------------------------------------------------------------ */
/*  KeyCombo — captured key combination                                */
/* ------------------------------------------------------------------ */
struct KeyCombo {
    UINT    vkCode;     /* Windows VK code (0 if standalone modifier) */
    uint8_t modifiers;  /* KCMOD_* bitmask                            */
};

/* ------------------------------------------------------------------ */
/*  Utility functions                                                  */
/* ------------------------------------------------------------------ */

/* Windows VK code → human-readable name (e.g. "F6", "Enter", "A") */
std::string vkToName(UINT vk);

/* Modifier bitmask → prefix string (e.g. "LCtrl+LAlt+") */
std::string modBitsToString(uint8_t modBits);

/* Full combo display name (e.g. "LCtrl+LAlt+F6", "LShift") */
std::string comboToString(const KeyCombo& combo);

/* Read currently held modifier keys via GetAsyncKeyState */
uint8_t getAsyncModifiers();

/* Check if a VK code is a modifier key */
bool isModifierVk(UINT vk);

/* Get KCMOD_* bit for a modifier VK (0 if not a modifier) */
uint8_t vkToModBit(UINT vk);

/* ------------------------------------------------------------------ */
/*  KeyCapture component                                               */
/* ------------------------------------------------------------------ */
class KeyCapture : public UIComponent {
public:
    KeyCapture(int x, int y, int w, int h,
               std::function<void(KeyCapture*, const KeyCombo&)> onChange = nullptr);
    ~KeyCapture() override;

    void create(HWND parent) override;
    int  getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }

    /* Get / set the current combo */
    void     setCombo(const KeyCombo& combo);
    void     setCombo(uint8_t modifiers, UINT vkCode);
    KeyCombo getCombo() const { return m_combo; }

    bool isListening() const { return m_listening; }

    /* Owner-draw rendering (called by SimpleWindow automatically) */
    bool handleDrawItem(DRAWITEMSTRUCT* dis) override;

    /* Styling */
    void setBackColor(COLORREF color);
    void setTextColor(COLORREF color);
    void setActiveColor(COLORREF color);
    void setActiveBorderColor(COLORREF color);
    void setBorderColor(COLORREF color);
    void setFont(const wchar_t* fontName, int size, bool bold = false);

    /* Custom display name formatter (overrides default comboToString) */
    void setComboFormatter(std::function<std::string(const KeyCombo&)> fmt);

private:
    int m_x, m_y, m_w, m_h;
    HWND m_hwnd = NULL;
    int m_id;
    static int s_nextId;

    KeyCombo m_combo = {0, 0};
    bool     m_listening  = false;
    uint8_t  m_pendingMod = 0;
    UINT     m_lastModVk  = 0;

    std::function<void(KeyCapture*, const KeyCombo&)> m_onChange;
    std::function<std::string(const KeyCombo&)> m_comboFormatter;

    /* Styling */
    COLORREF m_backColor       = CLR_INVALID;
    COLORREF m_textColor       = CLR_INVALID;
    COLORREF m_activeColor     = CLR_INVALID;
    COLORREF m_activeBorderClr = CLR_INVALID;
    COLORREF m_borderColor     = CLR_INVALID;
    HFONT    m_hFont           = NULL;

    std::string getDisplayText() const;
    void commitCombo(uint8_t mod, UINT vk);
    void cancelCapture();

    static LRESULT CALLBACK SubclassProc(HWND, UINT, WPARAM, LPARAM,
                                         UINT_PTR, DWORD_PTR);
};

#endif // KEY_CAPTURE_H
