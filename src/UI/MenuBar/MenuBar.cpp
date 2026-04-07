// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

// ============================================================================
// MenuBar — Implementacja
// ============================================================================
#include "MenuBar.h"
#include <UI/SimpleWindow/SimpleWindow.h>

// ============================================================================
// ID auto-inkrementacja (zakres 9000+, nie koliduje z UI 1000-8999)
// ============================================================================
int MenuBar::s_nextId = 9000;

// ============================================================================
// PopupMenu
// ============================================================================

PopupMenu::PopupMenu(MenuBar& owner)
    : m_owner(owner), m_hMenu(CreatePopupMenu()) {}

int PopupMenu::addItem(const wchar_t* label, std::function<void()> onClick) {
    int id = m_owner.allocateId();
    AppendMenuW(m_hMenu, MF_STRING, id, label);
    m_owner.registerItem(id, { false, false, onClick, nullptr });
    return id;
}

int PopupMenu::addCheckItem(const wchar_t* label, bool initialState,
                            std::function<void(bool)> onToggle) {
    int id = m_owner.allocateId();
    UINT flags = MF_STRING | (initialState ? MF_CHECKED : MF_UNCHECKED);
    AppendMenuW(m_hMenu, flags, id, label);
    m_owner.registerItem(id, { true, initialState, nullptr, onToggle });
    return id;
}

void PopupMenu::addSeparator() {
    AppendMenuW(m_hMenu, MF_SEPARATOR, 0, NULL);
}

void PopupMenu::addSubmenu(const wchar_t* label,
                           std::function<void(PopupMenu&)> builder) {
    PopupMenu sub(m_owner);
    builder(sub);
    AppendMenuW(m_hMenu, MF_POPUP, (UINT_PTR)sub.m_hMenu, label);
}

// ============================================================================
// MenuBar
// ============================================================================

MenuBar::MenuBar() : m_hMenuBar(CreateMenu()) {}

void MenuBar::addMenu(const wchar_t* label,
                      std::function<void(PopupMenu&)> builder) {
    PopupMenu popup(*this);
    builder(popup);
    AppendMenuW(m_hMenuBar, MF_POPUP, (UINT_PTR)popup.m_hMenu, label);
}

void MenuBar::attachTo(SimpleWindow* window) {
    if (!window) return;
    window->setMenu(m_hMenuBar);
    window->onMenuCommand([this](int cmdId) {
        handleCommand(cmdId);
    });
}

HMENU MenuBar::getHandle() const {
    return m_hMenuBar;
}

void MenuBar::setChecked(int itemId, bool checked) {
    auto it = m_items.find(itemId);
    if (it != m_items.end() && it->second.isCheck) {
        it->second.checked = checked;
        CheckMenuItem(m_hMenuBar, itemId,
                      MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
    }
}

bool MenuBar::isChecked(int itemId) const {
    auto it = m_items.find(itemId);
    if (it != m_items.end() && it->second.isCheck)
        return it->second.checked;
    return false;
}

void MenuBar::setEnabled(int itemId, bool enabled) {
    EnableMenuItem(m_hMenuBar, itemId,
                   MF_BYCOMMAND | (enabled ? MF_ENABLED : MF_GRAYED));
}

void MenuBar::setLabel(int itemId, const wchar_t* label) {
    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = const_cast<LPWSTR>(label);
    SetMenuItemInfoW(m_hMenuBar, itemId, FALSE, &mii);
}

int MenuBar::allocateId() {
    return s_nextId++;
}

void MenuBar::registerItem(int id, const ItemEntry& entry) {
    m_items[id] = entry;
}

void MenuBar::handleCommand(int cmdId) {
    auto it = m_items.find(cmdId);
    if (it == m_items.end()) return;

    auto& item = it->second;
    if (item.isCheck) {
        item.checked = !item.checked;
        CheckMenuItem(m_hMenuBar, cmdId,
                      MF_BYCOMMAND | (item.checked ? MF_CHECKED : MF_UNCHECKED));
        if (item.onToggle) item.onToggle(item.checked);
    } else {
        if (item.onClick) item.onClick();
    }
}
