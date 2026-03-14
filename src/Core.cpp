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