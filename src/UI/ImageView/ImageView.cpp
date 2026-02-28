#include "ImageView.h"
#include "../../Util/StringUtils.h"
#include <windows.h>
#include <map>

/* --------------------------------------------------------------- */
/*  GDI+ flat API — dynamically loaded, no gdiplus.lib needed      */
/* --------------------------------------------------------------- */

// Forward‑declare only the types we actually use
typedef int      GpStatus;        // Gdiplus::Status
typedef void     GpBitmap;        // opaque handle
typedef UINT32   ARGB;

struct GdiplusStartupInput {
    UINT32      GdiplusVersion;
    void*       DebugEventCallback;
    BOOL        SuppressBackgroundThread;
    BOOL        SuppressExternalCodecs;
};

// Function‑pointer typedefs
typedef GpStatus (WINAPI *fn_GdiplusStartup)(ULONG_PTR*, const GdiplusStartupInput*, void*);
typedef void     (WINAPI *fn_GdiplusShutdown)(ULONG_PTR);
typedef GpStatus (WINAPI *fn_GdipCreateBitmapFromFile)(const WCHAR*, GpBitmap**);
typedef GpStatus (WINAPI *fn_GdipCreateBitmapFromStream)(IStream*, GpBitmap**);
typedef GpStatus (WINAPI *fn_GdipGetImageWidth)(GpBitmap*, UINT*);
typedef GpStatus (WINAPI *fn_GdipGetImageHeight)(GpBitmap*, UINT*);
typedef GpStatus (WINAPI *fn_GdipCreateHBITMAPFromBitmap)(GpBitmap*, HBITMAP*, ARGB);
typedef GpStatus (WINAPI *fn_GdipDisposeImage)(GpBitmap*);

// shlwapi — only SHCreateMemStream
typedef IStream* (WINAPI *fn_SHCreateMemStream)(const BYTE*, UINT);

// Module‑level DLL handles (released implicitly on process exit)
static HMODULE s_gdiplusDll  = NULL;
static HMODULE s_shlwapiDll  = NULL;

// Function pointers
static fn_GdiplusStartup                pGdiplusStartup             = NULL;
static fn_GdiplusShutdown               pGdiplusShutdown            = NULL;
static fn_GdipCreateBitmapFromFile      pGdipCreateBitmapFromFile   = NULL;
static fn_GdipCreateBitmapFromStream    pGdipCreateBitmapFromStream = NULL;
static fn_GdipGetImageWidth             pGdipGetImageWidth          = NULL;
static fn_GdipGetImageHeight            pGdipGetImageHeight         = NULL;
static fn_GdipCreateHBITMAPFromBitmap   pGdipCreateHBITMAPFromBitmap= NULL;
static fn_GdipDisposeImage              pGdipDisposeImage           = NULL;
static fn_SHCreateMemStream             pSHCreateMemStream          = NULL;

static bool s_gdipLoaded  = false;
static bool s_shlwapiLoaded = false;

static bool ensureGdiplus() {
    if (s_gdipLoaded) return true;
    s_gdiplusDll = LoadLibraryA("gdiplus.dll");
    if (!s_gdiplusDll) return false;

    pGdiplusStartup              = (fn_GdiplusStartup)             GetProcAddress(s_gdiplusDll, "GdiplusStartup");
    pGdiplusShutdown             = (fn_GdiplusShutdown)            GetProcAddress(s_gdiplusDll, "GdiplusShutdown");
    pGdipCreateBitmapFromFile    = (fn_GdipCreateBitmapFromFile)   GetProcAddress(s_gdiplusDll, "GdipCreateBitmapFromFile");
    pGdipCreateBitmapFromStream  = (fn_GdipCreateBitmapFromStream) GetProcAddress(s_gdiplusDll, "GdipCreateBitmapFromStream");
    pGdipGetImageWidth           = (fn_GdipGetImageWidth)          GetProcAddress(s_gdiplusDll, "GdipGetImageWidth");
    pGdipGetImageHeight          = (fn_GdipGetImageHeight)         GetProcAddress(s_gdiplusDll, "GdipGetImageHeight");
    pGdipCreateHBITMAPFromBitmap = (fn_GdipCreateHBITMAPFromBitmap)GetProcAddress(s_gdiplusDll, "GdipCreateHBITMAPFromBitmap");
    pGdipDisposeImage            = (fn_GdipDisposeImage)           GetProcAddress(s_gdiplusDll, "GdipDisposeImage");

    if (!pGdiplusStartup || !pGdiplusShutdown || !pGdipCreateBitmapFromFile ||
        !pGdipCreateBitmapFromStream || !pGdipGetImageWidth || !pGdipGetImageHeight ||
        !pGdipCreateHBITMAPFromBitmap || !pGdipDisposeImage) {
        FreeLibrary(s_gdiplusDll);
        s_gdiplusDll = NULL;
        return false;
    }

    s_gdipLoaded = true;
    return true;
}

static bool ensureShlwapi() {
    if (s_shlwapiLoaded) return true;
    s_shlwapiDll = LoadLibraryA("shlwapi.dll");
    if (!s_shlwapiDll) return false;

    // SHCreateMemStream is ordinal 12 in older SDKs, but also exported by name
    pSHCreateMemStream = (fn_SHCreateMemStream)GetProcAddress(s_shlwapiDll, "SHCreateMemStream");
    if (!pSHCreateMemStream) {
        // Fallback: try ordinal 12
        pSHCreateMemStream = (fn_SHCreateMemStream)GetProcAddress(s_shlwapiDll, MAKEINTRESOURCEA(12));
    }

    if (!pSHCreateMemStream) {
        FreeLibrary(s_shlwapiDll);
        s_shlwapiDll = NULL;
        return false;
    }

    s_shlwapiLoaded = true;
    return true;
}

// Inicjalizacja zmiennej statycznej
int ImageView::s_nextId = 8000;

// Mapa do przechowywania wskaźników na obiekty ImageView według ich HWND
static std::map<HWND, ImageView*> imageViewsByHwnd;

// Identyfikator klasy okna
static const wchar_t* IMAGE_VIEW_CLASS_NAME = L"JQB_ImageView_Class";
static ATOM s_imageViewClass = 0;

ImageView::ImageView(int x, int y, int width, int height)
    : m_x(x), m_y(y), m_width(width), m_height(height), 
      m_hwnd(NULL), m_hBitmap(NULL), m_imageWidth(0), m_imageHeight(0),
      m_scaleMode(ASPECT_FIT), m_hasBorder(true), 
      m_backgroundColor(RGB(255, 255, 255)) {
    m_id = s_nextId++;
}

ImageView::~ImageView() {
    cleanup();
    
    if (m_hwnd) {
        // Usuń kontrolkę z mapy i zniszcz ją
        imageViewsByHwnd.erase(m_hwnd);
        DestroyWindow(m_hwnd);
        m_hwnd = NULL;
    }
}

void ImageView::cleanup() {
    if (m_hBitmap) {
        DeleteObject(m_hBitmap);
        m_hBitmap = NULL;
    }
    
    m_imageWidth = 0;
    m_imageHeight = 0;
}

void ImageView::create(HWND parent) {
    // Zarejestruj klasę okna, jeśli jeszcze nie jest zarejestrowana
    if (s_imageViewClass == 0) {
        WNDCLASSEXW wcex = {0};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = ImageViewProc;
        wcex.hInstance = _core.hInstance;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wcex.lpszClassName = IMAGE_VIEW_CLASS_NAME;
        
        s_imageViewClass = RegisterClassExW(&wcex);
        if (s_imageViewClass == 0) {
            MessageBoxW(NULL, L"Nie udało się zarejestrować klasy okna dla kontrolki obrazu!", L"Błąd", MB_ICONERROR);
            return;
        }
    }
    
    // Utwórz okno kontrolki
    DWORD style = WS_CHILD | WS_VISIBLE;
    if (m_hasBorder) {
        style |= WS_BORDER;
    }
    
    m_hwnd = CreateWindowExW(
        0,                          // Rozszerzony styl okna
        IMAGE_VIEW_CLASS_NAME,      // Nazwa klasy
        NULL,                       // Tekst
        style,                      // Styl okna
        m_x, m_y,                   // Pozycja
        m_width, m_height,          // Rozmiar
        parent,                     // Uchwyt rodzica
        (HMENU)(INT_PTR)m_id,       // ID menu
        _core.hInstance,            // Instancja
        NULL                        // Dane użytkownika
    );
    
    if (!m_hwnd) {
        MessageBoxW(NULL, L"Nie udało się utworzyć kontrolki obrazu!", L"Błąd", MB_ICONERROR);
        return;
    }
    
    // Dodaj kontrolkę do mapy
    imageViewsByHwnd[m_hwnd] = this;
    
    // Ustaw dane obiektu w danych okna
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
}

LRESULT CALLBACK ImageView::ImageViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Pobierz wskaźnik do obiektu ImageView
    ImageView* imageView = (ImageView*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_PAINT: {
            if (imageView) {
                // Obsługa rysowania
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hwnd, &ps);
                
                // Wypełnij tło kolorem tła
                RECT rect;
                GetClientRect(hwnd, &rect);
                HBRUSH hBrush = CreateSolidBrush(imageView->m_backgroundColor);
                FillRect(hdc, &rect, hBrush);
                DeleteObject(hBrush);
                
                // Narysuj obraz, jeśli istnieje
                if (imageView->m_hBitmap) {
                    // Utwórz kompatybilny DC
                    HDC hdcMem = CreateCompatibleDC(hdc);
                    HGDIOBJ hOldBitmap = SelectObject(hdcMem, imageView->m_hBitmap);
                    
                    int srcWidth = imageView->m_imageWidth;
                    int srcHeight = imageView->m_imageHeight;
                    int destWidth = rect.right - rect.left;
                    int destHeight = rect.bottom - rect.top;
                    int x = 0, y = 0;
                    
                    // Oblicz wymiary zgodnie z trybem skalowania
                    switch (imageView->m_scaleMode) {
                        case ImageView::ACTUAL_SIZE:
                            // Bez skalowania - oryginalny rozmiar
                            destWidth = srcWidth;
                            destHeight = srcHeight;
                            // Wyśrodkuj obraz
                            x = (rect.right - rect.left - srcWidth) / 2;
                            y = (rect.bottom - rect.top - srcHeight) / 2;
                            if (x < 0) x = 0;
                            if (y < 0) y = 0;
                            break;
                            
                        case ImageView::STRETCH:
                            // Rozciągnij do pełnego rozmiaru kontrolki
                            x = 0;
                            y = 0;
                            destWidth = rect.right - rect.left;
                            destHeight = rect.bottom - rect.top;
                            break;
                            
                        case ImageView::ASPECT_FIT: {
                            // Zachowaj proporcje, dopasuj do kontrolki
                            double ratioX = (double)(rect.right - rect.left) / srcWidth;
                            double ratioY = (double)(rect.bottom - rect.top) / srcHeight;
                            double ratio = ratioX < ratioY ? ratioX : ratioY;
                            
                            destWidth = (int)(srcWidth * ratio);
                            destHeight = (int)(srcHeight * ratio);
                            
                            // Wyśrodkuj obraz
                            x = (rect.right - rect.left - destWidth) / 2;
                            y = (rect.bottom - rect.top - destHeight) / 2;
                            break;
                        }
                            
                        case ImageView::CENTER:
                            // Wyśrodkuj obraz, przytnij, jeśli jest za duży
                            destWidth = srcWidth;
                            destHeight = srcHeight;
                            x = (rect.right - rect.left - srcWidth) / 2;
                            y = (rect.bottom - rect.top - srcHeight) / 2;
                            break;
                    }
                    
                    // Rysuj obraz z przezroczystością
                    SetStretchBltMode(hdc, HALFTONE);
                    StretchBlt(
                        hdc, x, y, destWidth, destHeight,
                        hdcMem, 0, 0, srcWidth, srcHeight,
                        SRCCOPY
                    );
                    
                    // Zwolnij zasoby
                    SelectObject(hdcMem, hOldBitmap);
                    DeleteDC(hdcMem);
                }
                
                EndPaint(hwnd, &ps);
                return 0;
            }
            break;
        }
        
        case WM_ERASEBKGND:
            // Zapobiegnij migotaniu tła
            return 1;
            
        case WM_DESTROY:
            // Usuń dane obiektu z okna
            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            break;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool ImageView::loadFromFile(const char* filePath) {
    // Wyczyść istniejący obraz
    cleanup();
    
    if (!ensureGdiplus()) return false;
    
    // Konwersja ścieżki z UTF-8 na UTF-16
    std::wstring widePath = StringUtils::utf8ToWide(filePath);
    
    // Inicjalizacja GDI+
    GdiplusStartupInput gdiplusStartupInput = { 1, NULL, FALSE, FALSE };
    ULONG_PTR gdiplusToken = 0;
    GpStatus status = pGdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (status != 0) {  // 0 = Ok
        return false;
    }
    
    bool success = false;
    
    // Wczytaj obraz z pliku (flat API)
    GpBitmap* bitmap = NULL;
    if (pGdipCreateBitmapFromFile(widePath.c_str(), &bitmap) == 0 && bitmap) {
        UINT w = 0, h = 0;
        pGdipGetImageWidth(bitmap, &w);
        pGdipGetImageHeight(bitmap, &h);
        m_imageWidth  = (int)w;
        m_imageHeight = (int)h;
        
        // Konwertuj na HBITMAP (White = 0xFFFFFFFF)
        HBITMAP hbmp = NULL;
        if (pGdipCreateHBITMAPFromBitmap(bitmap, &hbmp, 0xFFFFFFFF) == 0 && hbmp) {
            m_hBitmap = hbmp;
            success = true;
            
            if (m_hwnd) {
                InvalidateRect(m_hwnd, NULL, TRUE);
                UpdateWindow(m_hwnd);
            }
        }
        pGdipDisposeImage(bitmap);
    }
    
    pGdiplusShutdown(gdiplusToken);
    return success;
}

bool ImageView::loadFromResource(int resourceId) {
    // Wyczyść istniejący obraz
    cleanup();
    
    // Załaduj bitmapę z zasobów
    m_hBitmap = (HBITMAP)LoadImage(_core.hInstance, MAKEINTRESOURCE(resourceId), 
                                 IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
    
    if (!m_hBitmap) {
        return false;
    }
    
    // Pobierz informacje o bitmapie
    BITMAP bm;
    GetObject(m_hBitmap, sizeof(bm), &bm);
    m_imageWidth = bm.bmWidth;
    m_imageHeight = bm.bmHeight;
    
    // Wymuś odświeżenie kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindow(m_hwnd);
    }
    
    return true;
}

bool ImageView::loadFromMemory(const void* data, size_t size) {
    // Wyczyść istniejący obraz
    cleanup();
    
    if (!ensureGdiplus() || !ensureShlwapi()) return false;
    
    // Inicjalizacja GDI+
    GdiplusStartupInput gdiplusStartupInput = { 1, NULL, FALSE, FALSE };
    ULONG_PTR gdiplusToken = 0;
    GpStatus status = pGdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
    if (status != 0) {
        return false;
    }
    
    bool success = false;
    
    // Utwórz strumień pamięci
    IStream* stream = pSHCreateMemStream((BYTE*)data, (UINT)size);
    
    if (stream) {
        // Wczytaj obraz ze strumienia (flat API)
        GpBitmap* bitmap = NULL;
        if (pGdipCreateBitmapFromStream(stream, &bitmap) == 0 && bitmap) {
            UINT w = 0, h = 0;
            pGdipGetImageWidth(bitmap, &w);
            pGdipGetImageHeight(bitmap, &h);
            m_imageWidth  = (int)w;
            m_imageHeight = (int)h;
            
            HBITMAP hbmp = NULL;
            if (pGdipCreateHBITMAPFromBitmap(bitmap, &hbmp, 0xFFFFFFFF) == 0 && hbmp) {
                m_hBitmap = hbmp;
                success = true;
                
                if (m_hwnd) {
                    InvalidateRect(m_hwnd, NULL, TRUE);
                    UpdateWindow(m_hwnd);
                }
            }
            pGdipDisposeImage(bitmap);
        }
        
        stream->Release();
    }
    
    pGdiplusShutdown(gdiplusToken);
    return success;
}

void ImageView::clear() {
    cleanup();
    
    // Wymuś odświeżenie kontrolki
    if (m_hwnd) {
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindow(m_hwnd);
    }
}

void ImageView::setScaleMode(ImageView::ScaleMode mode) {
    if (m_scaleMode != mode) {
        m_scaleMode = mode;
        
        // Wymuś odświeżenie kontrolki
        if (m_hwnd) {
            InvalidateRect(m_hwnd, NULL, TRUE);
            UpdateWindow(m_hwnd);
        }
    }
}

void ImageView::setBorderStyle(bool hasBorder) {
    if (m_hasBorder != hasBorder) {
        m_hasBorder = hasBorder;
        
        if (m_hwnd) {
            // Zmień styl okna
            DWORD style = GetWindowLong(m_hwnd, GWL_STYLE);
            if (m_hasBorder) {
                style |= WS_BORDER;
            } else {
                style &= ~WS_BORDER;
            }
            SetWindowLong(m_hwnd, GWL_STYLE, style);
            
            // Wymuś odświeżenie kontrolki
            SetWindowPos(m_hwnd, NULL, 0, 0, 0, 0, 
                      SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            InvalidateRect(m_hwnd, NULL, TRUE);
            UpdateWindow(m_hwnd);
        }
    }
}

void ImageView::setBackgroundColor(COLORREF color) {
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        
        // Wymuś odświeżenie kontrolki
        if (m_hwnd) {
            InvalidateRect(m_hwnd, NULL, TRUE);
            UpdateWindow(m_hwnd);
        }
    }
}

bool ImageView::hasImage() const {
    return (m_hBitmap != NULL);
}

int ImageView::getImageWidth() const {
    return m_imageWidth;
}

int ImageView::getImageHeight() const {
    return m_imageHeight;
}