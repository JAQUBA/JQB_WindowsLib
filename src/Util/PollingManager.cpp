// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
#include "PollingManager.h"

void PollingManager::addGroup(const std::string& name, DWORD intervalMs, std::function<void()> action) {
    Group g;
    g.name       = name;
    g.intervalMs = intervalMs;
    g.action     = std::move(action);
    g.nextTick   = GetTickCount();
    m_groups.push_back(g);
}

PollingManager::Group* PollingManager::find(const std::string& name) {
    for (auto& g : m_groups) if (g.name == name) return &g;
    return nullptr;
}

void PollingManager::setEnabled(const std::string& name, bool enabled) {
    if (auto* g = find(name)) {
        g->enabled  = enabled;
        g->nextTick = GetTickCount();
    }
}

void PollingManager::setInterval(const std::string& name, DWORD intervalMs) {
    if (auto* g = find(name)) g->intervalMs = intervalMs;
}

void PollingManager::trigger(const std::string& name) {
    if (auto* g = find(name)) if (g->action) g->action();
}

void PollingManager::tick() {
    DWORD now = GetTickCount();
    for (auto& g : m_groups) {
        if (!g.enabled || !g.action) continue;
        if ((LONG)(now - g.nextTick) >= 0) {
            g.action();
            g.nextTick = now + g.intervalMs;
        }
    }
}
