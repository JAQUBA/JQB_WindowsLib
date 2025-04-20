#ifndef WINDOWS_APP_H
#define WINDOWS_APP_H

#include <windows.h>
#include <string>
#include <functional>
#include <map>
#include <vector>

namespace WinAppLib {

/**
 * @brief Klasa bazowa do tworzenia aplikacji okienkowych.
 * 
 * Umożliwia łatwe tworzenie i zarządzanie oknami aplikacji Windows
 * z prostym interfejsem w stylu obiektowym.
 */
class WindowsApp {
public:
    /**
     * @brief Konstruktor klasy WindowsApp
     * 
     * @param hInstance Uchwyt instancji aplikacji
     * @param appName Nazwa aplikacji
     * @param windowTitle Tytuł głównego okna aplikacji
     */
    WindowsApp(HINSTANCE hInstance, const std::wstring& appName, const std::wstring& windowTitle);
    
    /**
     * @brief Wirtualny destruktor
     */
    virtual ~WindowsApp();

    /**
     * @brief Inicjalizuje aplikację
     * 
     * @return true jeśli inicjalizacja się powiodła, false w przeciwnym przypadku
     */
    virtual bool initialize();
    
    /**
     * @brief Uruchamia główną pętlę aplikacji
     * 
     * @param nCmdShow Parametr nCmdShow z funkcji WinMain
     * @return Kod zakończenia aplikacji
     */
    int run(int nCmdShow);

    /**
     * @brief Pobiera uchwyt głównego okna aplikacji
     * 
     * @return Uchwyt okna (HWND)
     */
    HWND getMainWindowHandle() const { return m_hwnd; }
    
    /**
     * @brief Pobiera instancję aplikacji
     * 
     * @return Instancja aplikacji (HINSTANCE)
     */
    HINSTANCE getInstance() const { return m_hInstance; }
    
    /**
     * @brief Typ funkcji obsługującej komunikaty
     */
    using MessageHandlerFunc = std::function<bool(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT& result)>;

    /**
     * @brief Rejestruje obsługę konkretnego komunikatu Windows
     * 
     * @param msg Identyfikator komunikatu (np. WM_PAINT)
     * @param handler Funkcja obsługująca komunikat
     */
    void registerMessageHandler(UINT msg, MessageHandlerFunc handler);

protected:
    /**
     * @brief Metoda wywoływana podczas tworzenia głównego okna
     * 
     * @param hwnd Uchwyt do tworzonego okna
     * @return true jeśli inicjalizacja się powiodła, false w przeciwnym przypadku
     */
    virtual bool onCreate(HWND hwnd);
    
    /**
     * @brief Metoda wywoływana przy zamykaniu aplikacji
     */
    virtual void onDestroy();
    
    /**
     * @brief Metoda tworząca kontrolki UI
     */
    virtual void createControls();
    
    /**
     * @brief Metoda konfigurująca klasę okna
     * 
     * @param wc Referencja do struktury WNDCLASSEX do dostosowania
     */
    virtual void setupWindowClass(WNDCLASSEX& wc);
    
    /**
     * @brief Metoda konfigurująca parametry okna przy jego tworzeniu
     * 
     * @param style Referencja do stylu okna
     * @param exStyle Referencja do rozszerzonego stylu okna
     * @param x Referencja do pozycji X okna
     * @param y Referencja do pozycji Y okna
     * @param width Referencja do szerokości okna
     * @param height Referencja do wysokości okna
     */
    virtual void setupWindowParams(DWORD& style, DWORD& exStyle, int& x, int& y, int& width, int& height);

private:
    // Struktury wewnętrzne
    struct WindowClassRegistration {
        std::wstring className;
        HINSTANCE hInstance;
        
        // Operator porównania do użycia w mapie
        bool operator<(const WindowClassRegistration& other) const {
            if (hInstance != other.hInstance)
                return hInstance < other.hInstance;
            return className < other.className;
        }
    };

    // Statyczne składowe klasy
    static std::map<WindowClassRegistration, ATOM> s_registeredClasses;
    
    // Statyczny callback dla okna, przekierowujący do instancji klasy
    static LRESULT CALLBACK staticWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    // Procedura obsługi komunikatów okna
    LRESULT windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    
    // Rejestracja klasy okna
    ATOM registerWindowClass();

protected:
    // Pola instancji
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    std::wstring m_appName;
    std::wstring m_windowTitle;
    std::map<UINT, MessageHandlerFunc> m_messageHandlers;
};

} // namespace WinAppLib

// Makro ułatwiające definicję funkcji WinMain
#define WINAPP_ENTRY_POINT(AppClass)                                      \
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,          \
                   LPSTR lpCmdLine, int nCmdShow) {                       \
    AppClass app(hInstance);                                              \
    if (!app.initialize()) {                                              \
        MessageBox(NULL, L"Nie udało się zainicjalizować aplikacji",      \
                  L"Błąd", MB_OK | MB_ICONERROR);                         \
        return 0;                                                         \
    }                                                                     \
    return app.run(nCmdShow);                                             \
}

#endif // WINDOWS_APP_H