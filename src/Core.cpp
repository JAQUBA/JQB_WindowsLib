// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2026 JAQUBA (https://github.com/JAQUBA)
// Part of JQB_WindowsLib — https://github.com/JAQUBA/JQB_WindowsLib

#include "Core.h"

Core::Core() {
	init();
}
Core _core;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    
    _core.hInstance = hInstance;
    _core.hPrevInstance = hPrevInstance;
    _core.lpCmdLine = lpCmdLine;
    _core.nCmdShow = nCmdShow;

    setup();
    MSG msg = {};
    while (true) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return (int)msg.wParam;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        loop();
        Sleep(1);
    }
    
    return (int)msg.wParam;
}

__weak void init() {}
__weak void setup() {}
__weak void loop() {}