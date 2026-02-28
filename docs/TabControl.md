# TabControl — Kontrolka zakładek

> `#include <UI/TabControl/TabControl.h>`

## Opis

Kontrolka zakładek z panelami. Każda zakładka ma osobny panel (`HWND`), na którym można umieszczać inne komponenty. Obsługuje:
- Dodawanie/usuwanie zakładek
- Callback przy zmianie zakładki
- Automatyczne ukrywanie/pokazywanie paneli

## Konstruktor

```cpp
TabControl(int x, int y, int width, int height);
```

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę `WC_TABCONTROL` |
| `addTab(const char* title)` | `int` | Dodaje zakładkę, zwraca indeks |
| `getTabPage(int index)` | `HWND` | Uchwyt panelu zakładki (do umieszczania kontrolek) |
| `removeTab(int index)` | `void` | Usuwa zakładkę |
| `selectTab(int index)` | `void` | Programowy wybór zakładki |
| `getSelectedTab()` | `int` | Indeks aktywnej zakładki |
| `setTabTitle(int index, const char* title)` | `void` | Zmienia tytuł zakładki |
| `getTabTitle(int index)` | `std::string` | Pobiera tytuł |
| `getTabCount()` | `int` | Liczba zakładek |
| `onTabChange(function<void(int)> cb)` | `void` | Callback zmiany zakładki |
| `handleSelection()` | `void` | Wewnętrzna obsługa `TCN_SELCHANGE` |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 7000) |

## Przykłady

### Zakładki z panelami

```cpp
TabControl* tabs = new TabControl(10, 10, 580, 350);
window->add(tabs);

int tab1 = tabs->addTab("Ustawienia");
int tab2 = tabs->addTab("Log");
int tab3 = tabs->addTab("O programie");

// Pobierz uchwyt panelu i dodaj kontrolki
HWND page1 = tabs->getTabPage(tab1);

// Tworzenie kontrolek na panelu zakładki
CreateWindowW(L"BUTTON", L"Przycisk na Tab 1",
    WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
    20, 20, 120, 30,
    page1, NULL, _core.hInstance, NULL);
```

### Callback zmiany zakładki

```cpp
tabs->onTabChange([](int selectedIndex) {
    // selectedIndex — indeks nowo wybranej zakładki
    switch (selectedIndex) {
        case 0: /* Zakładka 1 */ break;
        case 1: /* Zakładka 2 */ break;
        case 2: /* Zakładka 3 */ break;
    }
});
```

### Programowa zmiana zakładki

```cpp
tabs->selectTab(1);  // Przełącz na drugą zakładkę
```

### Dynamiczne zarządzanie

```cpp
tabs->setTabTitle(0, "Nowy tytuł");
tabs->removeTab(2);
int count = tabs->getTabCount();
```

## Architektura paneli

Każda zakładka ma osobny panel (kontrolka `STATIC` z białym tłem), który jest dzieckiem kontrolki TabControl:

```
TabControl (WC_TABCONTROL)
├── Tab 0: "Ustawienia"  →  Panel HWND  (STATIC, SS_WHITERECT)
│                            ├── Button
│                            └── Label
├── Tab 1: "Log"         →  Panel HWND
│                            └── TextArea
└── Tab 2: "Info"        →  Panel HWND
                             └── Label
```

Tylko aktywny panel jest widoczny (`SW_SHOW`), reszta jest ukryta (`SW_HIDE`).

## Uwagi

- ID zaczynają się od **7000**
- Panele zakładek to kontrolki `STATIC` z `SS_WHITERECT`
- `handleSelection()` jest wywoływane przez `SimpleWindow::WindowProc` przy `WM_NOTIFY` + `TCN_SELCHANGE`
- Aby umieszczać komponenty JQB na panelu zakładki, użyj `getTabPage()` jako rodzica w `create()`
- Tytuły obsługują polskie znaki (UTF-8 → UTF-16, z fallback na ANSI)
