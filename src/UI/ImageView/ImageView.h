#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <functional>

class ImageView : public UIComponent {
public:
    // Tryby skalowania obrazu
    enum ScaleMode {
        ACTUAL_SIZE,    // Oryginalny rozmiar
        STRETCH,        // Rozciągnij do rozmiaru kontrolki
        ASPECT_FIT,     // Zachowaj proporcje, dopasuj do kontrolki
        CENTER          // Wyśrodkuj, przytnij jeśli zbyt duży
    };
    
    ImageView(int x, int y, int width, int height);
    ~ImageView();
    
    void create(HWND parent) override;
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }

    // Metody do wczytywania obrazów
    bool loadFromFile(const char* filePath);
    bool loadFromResource(int resourceId);
    bool loadFromMemory(const void* data, size_t size);
    
    // Metody do manipulacji obrazem
    void clear();
    void setScaleMode(ScaleMode mode);
    void setBorderStyle(bool hasBorder);
    void setBackgroundColor(COLORREF color);
    
    // Metody informacyjne
    bool hasImage() const;
    int getImageWidth() const;
    int getImageHeight() const;

private:
    // Funkcja do obsługi komunikatów okna
    static LRESULT CALLBACK ImageViewProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // Funkcja pomocnicza do czyszczenia zasobów
    void cleanup();
    
    int m_x, m_y, m_width, m_height;
    HWND m_hwnd;
    int m_id;
    
    HBITMAP m_hBitmap;          // Uchwyt do bitmapy
    int m_imageWidth;           // Szerokość obrazu
    int m_imageHeight;          // Wysokość obrazu
    ScaleMode m_scaleMode;      // Tryb skalowania
    bool m_hasBorder;           // Czy ma ramkę
    COLORREF m_backgroundColor; // Kolor tła
    
    static int s_nextId;
};

#endif // IMAGEVIEW_H