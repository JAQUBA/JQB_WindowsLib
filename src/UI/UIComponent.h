#ifndef UI_COMPONENT_H
#define UI_COMPONENT_H

#include "Core.h"

// Interfejs bazowy dla wszystkich komponentów UI
class UIComponent {
public:
    virtual ~UIComponent() = default;
    
    // Metody, które muszą być zaimplementowane przez wszystkie komponenty
    virtual void create(HWND parent) = 0;
    virtual int getId() const = 0;
    virtual HWND getHandle() const = 0;

    // Opcjonalne metody obsługi zdarzeń
    virtual void handleClick() {}
    virtual void handleSelection() {}

    // Owner-draw rendering (return true if handled)
    virtual bool handleDrawItem(DRAWITEMSTRUCT*) { return false; }
};

#endif // UI_COMPONENT_H