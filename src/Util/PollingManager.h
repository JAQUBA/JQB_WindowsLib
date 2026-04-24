// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib
//
// PollingManager — periodic polling groups, ticked from loop().
//
// Each group has a name, an interval and an action callback. Call tick()
// from loop() once per cycle; groups whose interval has elapsed and which
// are enabled fire their action. Generic — no protocol dependency.
#ifndef JQB_POLLING_MANAGER_H
#define JQB_POLLING_MANAGER_H

#include "Core.h"
#include <functional>
#include <vector>
#include <string>

class PollingManager {
public:
    struct Group {
        std::string           name;
        DWORD                 intervalMs = 1000;
        bool                  enabled    = false;
        DWORD                 nextTick   = 0;
        std::function<void()> action;
    };

    void   addGroup(const std::string& name, DWORD intervalMs, std::function<void()> action);
    Group* find    (const std::string& name);
    void   setEnabled (const std::string& name, bool enabled);
    void   setInterval(const std::string& name, DWORD intervalMs);
    void   trigger    (const std::string& name); // run action once now (if registered)
    void   tick();                               // call from loop()

private:
    std::vector<Group> m_groups;
};

#endif // JQB_POLLING_MANAGER_H
