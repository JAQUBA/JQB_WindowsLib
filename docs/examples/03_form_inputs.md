# Example 03 — Form Inputs

Demo form with InputField, CheckBox, Select, and a submit button.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/InputField/InputField.h>
#include <UI/CheckBox/CheckBox.h>
#include <UI/Select/Select.h>
#include <UI/TextArea/TextArea.h>
#include <Util/StringUtils.h>
#include <Util/ConfigManager.h>

SimpleWindow*  window;
InputField*    inputName;
InputField*    inputPassword;
CheckBox*      chkRemember;
Select*        selRole;
TextArea*      txtResult;
ConfigManager  config("form_settings.ini");

void setup() {
    window = new SimpleWindow(500, 450, "Form", 0);
    window->init();

    int y = 15;
    const int LX = 15, SX = 150, LW = 130, SW = 320;

    // --- Name ---
    window->add(new Label(LX, y, LW, 22, L"Name:"));
    inputName = new InputField(SX, y, SW, 25, "", nullptr);
    inputName->setPlaceholder("Enter name...");
    inputName->setMaxLength(50);
    window->add(inputName);
    y += 35;

    // --- Password ---
    window->add(new Label(LX, y, LW, 22, L"Password:"));
    inputPassword = new InputField(SX, y, SW, 25, "", nullptr);
    inputPassword->setPlaceholder("Enter password...");
    inputPassword->setPasswordMode(true);
    window->add(inputPassword);
    y += 35;

    // --- Role ---
    window->add(new Label(LX, y, LW, 22, L"Role:"));
    selRole = new Select(SX, y, SW, 200, "User", [](Select* sel) {
        // Optional callback on change
    });
    window->add(selRole);
    selRole->addItem("Administrator");
    selRole->addItem("User");
    selRole->addItem("Guest");
    y += 35;

    // --- Checkbox ---
    chkRemember = new CheckBox(SX, y, SW, 22, "Remember settings", false,
        [](CheckBox* cb, bool checked) {
            // Immediate reaction (optional)
        });
    window->add(chkRemember);
    y += 35;

    // --- Submit button ---
    window->add(new Button(SX, y, 150, 32, "Submit", [](Button*) {
        const char* name = inputName->getText();
        const char* pass = inputPassword->getText();
        const char* role = selRole->getText();
        bool remember = chkRemember->isChecked();

        // Display summary
        std::string summary;
        summary += "Name: ";
        summary += (name ? name : "(empty)");
        summary += "\r\nRole: ";
        summary += (role ? role : "(none)");
        summary += "\r\nRemember: ";
        summary += (remember ? "YES" : "NO");
        summary += "\r\n";

        txtResult->setText(StringUtils::utf8ToWide(summary).c_str());

        // Save to config if checkbox is checked
        if (remember) {
            config.setValue("name", name ? name : "");
            config.setValue("role", role ? role : "");
        }
    }));

    window->add(new Button(SX + 160, y, 150, 32, "Clear", [](Button*) {
        inputName->setText("");
        inputPassword->setText("");
        txtResult->setText(L"");
    }));
    y += 45;

    // --- Result ---
    window->add(new Label(LX, y, LW, 22, L"Result:"));
    txtResult = new TextArea(SX, y, SW, 120, nullptr);
    window->add(txtResult);

    // --- Load saved values ---
    std::string savedName = config.getValue("name", "");
    std::string savedRole = config.getValue("role", "");
    if (!savedName.empty()) {
        inputName->setText(savedName.c_str());
    }
}

void loop() {}
```

## Key Points

1. **InputField** — `setPlaceholder()` sets hint text, `setPasswordMode(true)` masks characters
2. **InputField::setMaxLength(n)** — character limit
3. **CheckBox** — constructor: `(x, y, w, h, text, defaultState, callback)`
4. **CheckBox::isChecked()** — reads the current state
5. **Select::addItem()** — adds items after creation
6. **ConfigManager** — `setValue()` / `getValue()` for persistent settings storage
7. **TextArea** — readonly, ideal for displaying results
