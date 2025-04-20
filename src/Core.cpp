#include "Core.h"

Core::Core() {
	init();
}
Core _core;

// Funkcja wWinMain dla wersji Unicode
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    setup();
    
    // Główna pętla komunikatów aplikacji
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        // Jeśli otrzymano wiadomość WM_QUIT, zakończ działanie aplikacji
        if (msg.message == WM_QUIT) {
            break;
        }
        
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        
        // Wywołaj funkcję loop z głównego pliku
        loop();
    }
    
    return (int)msg.wParam;
}

// Dodanie funkcji WinMain dla wersji ANSI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    return wWinMain(hInstance, hPrevInstance, NULL, nCmdShow);
}