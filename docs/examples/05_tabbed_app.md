# Przykład 05 — Tabbed App

Aplikacja z zakładkami — każda zakładka ma własny panel z kontrolkami.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/TabControl/TabControl.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/InputField/InputField.h>
#include <UI/CheckBox/CheckBox.h>
#include <Util/StringUtils.h>

SimpleWindow* window;
TabControl*   tabs;

// Pomocnicza funkcja do tworzenia kontrolek bezpośrednio na panelu zakładki
// (kontrolki na zakładkach NIE są zarządzane przez SimpleWindow — użyj WinAPI)
HWND createStaticLabel(HWND parent, int x, int y, int w, int h, const wchar_t* text) {
    return CreateWindowW(L"STATIC", text,
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y, w, h, parent, NULL, _core.hInstance, NULL);
}

HWND createButton(HWND parent, int x, int y, int w, int h, const wchar_t* text, int id) {
    return CreateWindowW(L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, _core.hInstance, NULL);
}

void setup() {
    window = new SimpleWindow(600, 400, "Tabbed Application", 0);
    window->init();

    // --- TabControl ---
    tabs = new TabControl(10, 10, 570, 340);
    window->add(tabs);

    // Dodaj zakładki
    tabs->addTab("Główna");
    tabs->addTab("Ustawienia");
    tabs->addTab("O programie");

    // --- Zakładka 0: Główna ---
    HWND page0 = tabs->getTabPage(0);
    createStaticLabel(page0, 20, 20, 400, 25, L"Witaj na stronie głównej!");
    createStaticLabel(page0, 20, 55, 400, 25, L"Wybierz zakładkę powyżej.");

    // --- Zakładka 1: Ustawienia ---
    HWND page1 = tabs->getTabPage(1);
    createStaticLabel(page1, 20, 20, 150, 22, L"Nazwa urządzenia:");
    CreateWindowW(L"EDIT", L"MojeUrządzenie",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        180, 18, 250, 24, page1, NULL, _core.hInstance, NULL);

    createStaticLabel(page1, 20, 55, 150, 22, L"Tryb pracy:");
    HWND combo = CreateWindowW(L"COMBOBOX", NULL,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        180, 53, 250, 200, page1, NULL, _core.hInstance, NULL);
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Automatyczny");
    SendMessageW(combo, CB_ADDSTRING, 0, (LPARAM)L"Ręczny");
    SendMessageW(combo, CB_SETCURSEL, 0, 0);

    HWND chk = CreateWindowW(L"BUTTON", L"Włącz powiadomienia",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 90, 300, 22, page1, NULL, _core.hInstance, NULL);
    SendMessage(chk, BM_SETCHECK, BST_CHECKED, 0);

    createButton(page1, 20, 130, 120, 30, L"Zapisz", 9001);

    // --- Zakładka 2: O programie ---
    HWND page2 = tabs->getTabPage(2);
    createStaticLabel(page2, 20, 20, 500, 25, L"Tabbed App v1.0");
    createStaticLabel(page2, 20, 50, 500, 25, L"Zbudowane z JQB_WindowsLib");
    createStaticLabel(page2, 20, 80, 500, 25, L"github.com/JAQUBA/JQB_WindowsLib");

    // Callback zmiany zakładki (opcjonalnie)
    tabs->onTabChange([](int newIndex) {
        // Reaguj na zmianę zakładki
    });
}

void loop() {}
```

## Kluczowe punkty

1. **TabControl** — tworzy zakładki z osobnymi panelami HWND
   - `addTab("Nazwa")` — dodaje zakładkę
   - `getTabPage(index)` — zwraca HWND panelu zakładki
   - `onTabChange(callback)` — callback przy zmianie zakładki

2. **Kontrolki na zakładkach** — umieszczane bezpośrednio na HWND panelu
   - Użyj `CreateWindowW()` z WinAPI, podając `tabs->getTabPage(i)` jako parent
   - Kontrolki te **nie** są zarządzane przez `SimpleWindow::add()` — zarządzasz nimi sam

3. **`_core.hInstance`** — globalny HINSTANCE potrzebny do `CreateWindow`

4. **Alternatywa**: Możesz też tworzyć osobne `SimpleWindow` jako dialogi zamiast zakładek — zależy od złożoności UI
