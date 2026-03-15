# ImageView — Image Display

> `#include <UI/ImageView/ImageView.h>`

## Description

Image display control with support for:
- Loading from file (BMP, PNG, JPG, GIF — via GDI+)
- Loading from resources (`.rc`)
- Loading from memory (byte buffer)
- Scale modes (fit, stretch, center, original size)

## Constructor

```cpp
ImageView(int x, int y, int width, int height);
```

## Scale Modes (`ScaleMode`)

```cpp
enum ScaleMode {
    ACTUAL_SIZE,   // Original size — centered
    STRETCH,       // Stretched to control size
    ASPECT_FIT,    // Preserves aspect ratio, fits control (default)
    CENTER         // Centered, cropped if too large
};
```

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the image control |
| `loadFromFile(const char* path)` | `bool` | Loads from file (UTF-8, GDI+) |
| `loadFromResource(int id)` | `bool` | Loads from resources |
| `loadFromMemory(const void* data, size_t size)` | `bool` | Loads from buffer |
| `clear()` | `void` | Removes image |
| `setScaleMode(ScaleMode mode)` | `void` | Scale mode |
| `setBorderStyle(bool hasBorder)` | `void` | Border around control |
| `setBackgroundColor(COLORREF color)` | `void` | Background color |
| `hasImage()` | `bool` | Whether an image is loaded |
| `getImageWidth()` | `int` | Original image width |
| `getImageHeight()` | `int` | Original image height |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 8000) |

## Examples

### Image from File

```cpp
ImageView* img = new ImageView(20, 20, 200, 200);
window->add(img);

img->loadFromFile("photo.png");              // PNG
img->loadFromFile("C:\\images\\logo.jpg");    // JPG with full path
```

### Image from Resources

```cpp
// In resources/resources.rc: 101 BITMAP "logo.bmp"
ImageView* logo = new ImageView(10, 10, 100, 100);
window->add(logo);
logo->loadFromResource(101);
```

### Scale Mode

```cpp
img->setScaleMode(ImageView::STRETCH);       // Stretch
img->setScaleMode(ImageView::ASPECT_FIT);    // Proportional (default)
img->setScaleMode(ImageView::CENTER);        // Center
img->setScaleMode(ImageView::ACTUAL_SIZE);   // 1:1
```

### Background and Border

```cpp
img->setBackgroundColor(RGB(240, 240, 240));  // Light gray background
img->setBorderStyle(false);                    // No border
```

## Notes

- **GDI+** is initialized and shut down internally during load operations
- Supported formats: BMP, PNG, JPEG, GIF, TIFF (whatever GDI+ supports)
- `loadFromFile()` converts path from UTF-8 to UTF-16
- `loadFromMemory()` creates an `IStream` via `SHCreateMemStream` (requires `Shlwapi`)
- IDs start at **8000**
- Registers its own window class `JQB_ImageView_Class`
- `WM_ERASEBKGND` returns 1 — eliminates flicker
- Scaling: `StretchBlt` with `HALFTONE` for quality
