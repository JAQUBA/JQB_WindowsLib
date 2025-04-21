#include "Core.h"

Core::Core() {
	init();
}
Core _core;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    
    _core.hInstance = hInstance;
    _core.hPrevInstance = hPrevInstance;
    _core.lpCmdLine = lpCmdLine;
    _core.nCmdShow = nCmdShow;

    setup();
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) {
            break;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        loop();
    }
    
    return (int)msg.wParam;
}

__weak void init() {}
__weak void setup() {}
__weak void loop() {}