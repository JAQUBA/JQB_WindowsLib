# Przykład 07 — Image Viewer

Przeglądarka obrazów z GDI+ — ładowanie z pliku, zmiana trybu skalowania.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/Select/Select.h>
#include <UI/ImageView/ImageView.h>

#include <commdlg.h>  // GetOpenFileName

SimpleWindow* window;
ImageView*    imgView;
Label*        lblFile;
Select*       selScale;

// Otwieranie pliku dialogiem Windows
std::wstring openFileDialog(HWND owner) {
    wchar_t file[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = owner;
    ofn.lpstrFilter  = L"Obrazy (*.bmp;*.png;*.jpg;*.jpeg)\0*.bmp;*.png;*.jpg;*.jpeg\0"
                       L"Wszystkie pliki (*.*)\0*.*\0";
    ofn.lpstrFile    = file;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn))
        return std::wstring(file);
    return L"";
}

void setup() {
    window = new SimpleWindow(800, 620, "Image Viewer", 0);
    window->init();

    int y = 10;

    // --- Pasek narzędzi ---
    window->add(new Button(10, y, 120, 28, "Otwórz plik", [](Button*) {
        std::wstring path = openFileDialog(NULL);
        if (!path.empty()) {
            imgView->loadFromFile(path);
            lblFile->setText(path.c_str());
        }
    }));

    window->add(new Label(140, y + 3, 60, 22, L"Tryb:"));

    selScale = new Select(200, y, 160, 200, "Aspect Fit", [](Select* sel) {
        int idx = (int)SendMessage(sel->getHandle(), CB_GETCURSEL, 0, 0);
        switch (idx) {
            case 0: imgView->setScaleMode(ImageView::ScaleMode::ASPECT_FIT);  break;
            case 1: imgView->setScaleMode(ImageView::ScaleMode::ASPECT_FILL); break;
            case 2: imgView->setScaleMode(ImageView::ScaleMode::STRETCH);     break;
            case 3: imgView->setScaleMode(ImageView::ScaleMode::ORIGINAL);    break;
        }
    });
    window->add(selScale);
    selScale->addItem("Aspect Fit");
    selScale->addItem("Aspect Fill");
    selScale->addItem("Stretch");
    selScale->addItem("Original");

    lblFile = new Label(380, y + 3, 400, 22, L"(brak pliku)");
    window->add(lblFile);
    y += 38;

    // --- Widok obrazu ---
    imgView = new ImageView(10, y, 770, 540);
    window->add(imgView);
    imgView->setScaleMode(ImageView::ScaleMode::ASPECT_FIT);
}

void loop() {}
```

## Kluczowe punkty

1. **ImageView** — wyświetlanie obrazów z GDI+
   - `loadFromFile(wstring)` — ładuje BMP, PNG, JPG z dysku
   - `loadFromResource(resId)` — ładuje z zasobów RC
   - `loadFromMemory(data, size)` — ładuje z bufora w pamięci
   - `setScaleMode(mode)` — tryb skalowania:
     - `ASPECT_FIT` — mieści z zachowaniem proporcji (domyślny)
     - `ASPECT_FILL` — wypełnia z zachowaniem proporcji (obcina)
     - `STRETCH` — rozciąga do rozmiaru kontrolki
     - `ORIGINAL` — oryginalna wielkość (1:1)

2. **GDI+** — inicjalizowany automatycznie przez bibliotekę (nie musisz wywoływać `GdiplusStartup`)

3. **OPENFILENAMEW** — standardowy dialog Windows do wyboru pliku (z `commdlg.h`)

4. **linkowanie `comdlg32`** — może być potrzebne dodanie do `build_flags`:
   ```ini
   build_flags = -std=c++17 -lcomdlg32
   ```
