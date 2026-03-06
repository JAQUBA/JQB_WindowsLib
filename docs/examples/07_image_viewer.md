# Example 07 — Image Viewer

Image viewer using GDI+ — loading from file, switching scale modes.

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

// Open file using the Windows dialog
std::wstring openFileDialog(HWND owner) {
    wchar_t file[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = owner;
    ofn.lpstrFilter  = L"Images (*.bmp;*.png;*.jpg;*.jpeg)\0*.bmp;*.png;*.jpg;*.jpeg\0"
                       L"All files (*.*)\0*.*\0";
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

    // --- Toolbar ---
    window->add(new Button(10, y, 120, 28, "Open file", [](Button*) {
        std::wstring path = openFileDialog(NULL);
        if (!path.empty()) {
            imgView->loadFromFile(path);
            lblFile->setText(path.c_str());
        }
    }));

    window->add(new Label(140, y + 3, 60, 22, L"Mode:"));

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

    lblFile = new Label(380, y + 3, 400, 22, L"(no file)");
    window->add(lblFile);
    y += 38;

    // --- Image view ---
    imgView = new ImageView(10, y, 770, 540);
    window->add(imgView);
    imgView->setScaleMode(ImageView::ScaleMode::ASPECT_FIT);
}

void loop() {}
```

## Key Points

1. **ImageView** — image rendering with GDI+
   - `loadFromFile(wstring)` — loads BMP, PNG, JPG from disk
   - `loadFromResource(resId)` — loads from RC resources
   - `loadFromMemory(data, size)` — loads from a memory buffer
   - `setScaleMode(mode)` — scaling mode:
     - `ASPECT_FIT` — fits while preserving aspect ratio (default)
     - `ASPECT_FILL` — fills while preserving ratio (crops)
     - `STRETCH` — stretches to control size
     - `ORIGINAL` — original size (1:1)

2. **GDI+** — initialized automatically by the library (no need to call `GdiplusStartup`)

3. **OPENFILENAMEW** — standard Windows file open dialog (from `commdlg.h`)

4. **Linking `comdlg32`** — you may need to add to `build_flags`:
   ```ini
   build_flags = -lcomdlg32
   ```
