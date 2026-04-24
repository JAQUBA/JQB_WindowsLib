// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#ifndef TABCONTROL_H
#define TABCONTROL_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <vector>
#include <functional>

class TabControl : public UIComponent {
public:
    TabControl(int x, int y, int width, int height);
    ~TabControl();
    
    void create(HWND parent) override;
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }
    
    // Metody specyficzne dla TabControl
    int addTab(const char* title);
    HWND getTabPage(int index) const;
    void removeTab(int index);
    void selectTab(int index);
    int getSelectedTab() const;
    void setTabTitle(int index, const char* title);
    std::string getTabTitle(int index) const;
    int getTabCount() const;
    
    // Metoda do obsługi zmiany zakładki
    void onTabChange(std::function<void(int)> callback);
    
    // Obsługa zdarzenia zmiany zakładki
    void handleSelection() override;

    // Theming — set background color for tab content pages.
    // Call after addTab(); recolors all existing pages and any added later.
    // Used by applyTheme() to make tab pages match the window theme.
    void setPageBackground(COLORREF color);
    COLORREF getPageBackground() const { return m_pageBg; }

private:
    int m_x, m_y, m_width, m_height;
    HWND m_hwnd;
    int m_id;
    std::vector<HWND> m_tabPages;
    std::function<void(int)> m_onTabChangeCallback;
    COLORREF m_pageBg = CLR_INVALID;
    HBRUSH   m_pageBrush = NULL;

    void applyPageBrush(HWND hPage);

    static int s_nextId;
};

#endif // TABCONTROL_H