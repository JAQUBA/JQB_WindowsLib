// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// MenuBar — Uniwersalny pasek menu z callbackami (lambda)
// ============================================================================
#pragma once

#include <windows.h>
#include <functional>
#include <map>

class SimpleWindow;
class MenuBar;

// ============================================================================
// PopupMenu — podmenu budowane w lambdzie przekazanej do addMenu()
// ============================================================================
class PopupMenu {
    friend class MenuBar;
public:
    int  addItem(const wchar_t* label, std::function<void()> onClick);
    int  addCheckItem(const wchar_t* label, bool initialState,
                      std::function<void(bool)> onToggle);
    void addSeparator();
    void addSubmenu(const wchar_t* label,
                    std::function<void(PopupMenu&)> builder);

private:
    PopupMenu(MenuBar& owner);
    HMENU    m_hMenu;
    MenuBar& m_owner;
};

// ============================================================================
// MenuBar — pasek menu okna z auto-ID i auto-toggle
// ============================================================================
class MenuBar {
    friend class PopupMenu;
public:
    MenuBar();

    void addMenu(const wchar_t* label,
                 std::function<void(PopupMenu&)> builder);
    void attachTo(SimpleWindow* window);

    HMENU getHandle() const;

    void setChecked(int itemId, bool checked);
    bool isChecked(int itemId) const;
    void setEnabled(int itemId, bool enabled);
    void setLabel(int itemId, const wchar_t* label);

private:
    HMENU m_hMenuBar;

    struct ItemEntry {
        bool isCheck;
        bool checked;
        std::function<void()> onClick;
        std::function<void(bool)> onToggle;
    };

    std::map<int, ItemEntry> m_items;
    static int s_nextId;

    int  allocateId();
    void registerItem(int id, const ItemEntry& entry);
    void handleCommand(int cmdId);
};
