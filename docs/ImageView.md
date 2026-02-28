# ImageView — Wyświetlanie obrazów

> `#include <UI/ImageView/ImageView.h>`

## Opis

Kontrolka do wyświetlania obrazów z obsługą:
- Wczytywanie z pliku (BMP, PNG, JPG, GIF — przez GDI+)
- Wczytywanie z zasobów (`.rc`)
- Wczytywanie z pamięci (bufor bajtów)
- Tryby skalowania (dopasuj, rozciągnij, wyśrodkuj, oryginalny rozmiar)

## Konstruktor

```cpp
ImageView(int x, int y, int width, int height);
```

## Tryby skalowania (`ScaleMode`)

```cpp
enum ScaleMode {
    ACTUAL_SIZE,   // Oryginalny rozmiar — wyśrodkowany
    STRETCH,       // Rozciągnięty do rozmiaru kontrolki
    ASPECT_FIT,    // Zachowuje proporcje, dopasowany do kontrolki (domyślny)
    CENTER         // Wyśrodkowany, przycinany jeśli za duży
};
```

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `create(HWND parent)` | `void` | Tworzy kontrolkę obrazu |
| `loadFromFile(const char* path)` | `bool` | Wczytuje z pliku (UTF-8, GDI+) |
| `loadFromResource(int id)` | `bool` | Wczytuje z zasobów |
| `loadFromMemory(const void* data, size_t size)` | `bool` | Wczytuje z bufora |
| `clear()` | `void` | Usuwa obraz |
| `setScaleMode(ScaleMode mode)` | `void` | Tryb skalowania |
| `setBorderStyle(bool hasBorder)` | `void` | Ramka wokół kontrolki |
| `setBackgroundColor(COLORREF color)` | `void` | Kolor tła |
| `hasImage()` | `bool` | Czy obraz jest załadowany |
| `getImageWidth()` | `int` | Szerokość oryginału |
| `getImageHeight()` | `int` | Wysokość oryginału |
| `getHandle()` | `HWND` | Uchwyt kontrolki |
| `getId()` | `int` | Unikalny ID (auto od 8000) |

## Przykłady

### Obraz z pliku

```cpp
ImageView* img = new ImageView(20, 20, 200, 200);
window->add(img);

img->loadFromFile("photo.png");              // PNG
img->loadFromFile("C:\\images\\logo.jpg");    // JPG z pełną ścieżką
```

### Obraz z zasobów

```cpp
// W resources.rc: 101 BITMAP "logo.bmp"
ImageView* logo = new ImageView(10, 10, 100, 100);
window->add(logo);
logo->loadFromResource(101);
```

### Tryb skalowania

```cpp
img->setScaleMode(ImageView::STRETCH);       // Rozciągnij
img->setScaleMode(ImageView::ASPECT_FIT);    // Proporcjonalnie (domyślny)
img->setScaleMode(ImageView::CENTER);        // Wyśrodkuj
img->setScaleMode(ImageView::ACTUAL_SIZE);   // 1:1
```

### Tło i ramka

```cpp
img->setBackgroundColor(RGB(240, 240, 240));  // Jasnoszare tło
img->setBorderStyle(false);                    // Bez ramki
```

## Uwagi

- **GDI+** jest inicjowane i zamykane wewnętrznie podczas operacji ładowania
- Obsługiwane formaty: BMP, PNG, JPEG, GIF, TIFF (co obsługuje GDI+)
- `loadFromFile()` konwertuje ścieżkę z UTF-8 na UTF-16
- `loadFromMemory()` tworzy `IStream` przez `SHCreateMemStream` (wymaga `Shlwapi`)
- ID zaczynają się od **8000**
- Rejestruje własną klasę okna `JQB_ImageView_Class`
- `WM_ERASEBKGND` zwraca 1 — eliminuje migotanie
- Skalowanie: `StretchBlt` z `HALFTONE` dla jakości
