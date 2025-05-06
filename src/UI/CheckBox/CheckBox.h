#ifndef CHECKBOX_H
#define CHECKBOX_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <functional>

class CheckBox : public UIComponent {
public:
    CheckBox(int x, int y, int width, int height, const char* text, bool checked = false,
             std::function<void(CheckBox*, bool)> onChange = nullptr);
    ~CheckBox();
    
    void create(HWND parent) override;
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }

    // Metody specyficzne dla CheckBox
    void setText(const char* text);
    std::string getText() const;
    void setChecked(bool checked);
    bool isChecked() const;
    
    // Obsługa zdarzeń
    void handleClick() override;
    void handleStateChange();

private:
    int m_x, m_y, m_width, m_height;
    std::string m_text;
    bool m_checked;
    HWND m_hwnd;
    int m_id;
    
    std::function<void(CheckBox*, bool)> m_onChangeCallback;
    
    static int s_nextId;
};

#endif // CHECKBOX_H