#ifndef INPUTFIELD_H
#define INPUTFIELD_H

#include "Core.h"
#include "../UIComponent.h"
#include <string>
#include <functional>

class InputField : public UIComponent {
public:
    InputField(int x, int y, int width, int height, const char* defaultText = "",
               std::function<void(InputField*, const char*)> onTextChange = nullptr);
    ~InputField();

    void create(HWND parent) override;
    int getId() const override { return m_id; }
    HWND getHandle() const override { return m_hwnd; }

    // Metody specyficzne dla InputField
    void setText(const char* text);
    std::string getText() const;
    void setPlaceholder(const char* placeholderText);
    void setMaxLength(int maxLength);
    void setReadOnly(bool readOnly);
    void setPassword(bool isPassword);
    
    // Obsługa zdarzeń
    void handleTextChange();

private:
    int m_x, m_y, m_width, m_height;
    std::string m_text;
    std::string m_placeholder;
    int m_maxLength;
    bool m_readOnly;
    bool m_isPassword;
    HWND m_hwnd;
    int m_id;
    
    std::function<void(InputField*, const char*)> m_onTextChangeCallback;
    
    static int s_nextId;
};

#endif // INPUTFIELD_H