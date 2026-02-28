# Przykład 03 — Form Inputs

Formularz demonstracyjny z InputField, CheckBox, Select i przyciskiem zatwierdzenia.

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
    window = new SimpleWindow(500, 450, "Formularz", 0);
    window->init();

    int y = 15;
    const int LX = 15, SX = 150, LW = 130, SW = 320;

    // --- Imię ---
    window->add(new Label(LX, y, LW, 22, L"Imię:"));
    inputName = new InputField(SX, y, SW, 25, "", nullptr);
    inputName->setPlaceholder("Wpisz imię...");
    inputName->setMaxLength(50);
    window->add(inputName);
    y += 35;

    // --- Hasło ---
    window->add(new Label(LX, y, LW, 22, L"Hasło:"));
    inputPassword = new InputField(SX, y, SW, 25, "", nullptr);
    inputPassword->setPlaceholder("Wpisz hasło...");
    inputPassword->setPasswordMode(true);
    window->add(inputPassword);
    y += 35;

    // --- Rola ---
    window->add(new Label(LX, y, LW, 22, L"Rola:"));
    selRole = new Select(SX, y, SW, 200, "Użytkownik", [](Select* sel) {
        // Opcjonalny callback przy zmianie
    });
    window->add(selRole);
    selRole->addItem("Administrator");
    selRole->addItem("Użytkownik");
    selRole->addItem("Gość");
    y += 35;

    // --- Checkbox ---
    chkRemember = new CheckBox(SX, y, SW, 22, "Zapamiętaj ustawienia", false,
        [](CheckBox* cb, bool checked) {
            // Reakcja natychmiastowa (opcjonalnie)
        });
    window->add(chkRemember);
    y += 35;

    // --- Przycisk zatwierdzenia ---
    window->add(new Button(SX, y, 150, 32, "Zatwierdź", [](Button*) {
        const char* name = inputName->getText();
        const char* pass = inputPassword->getText();
        const char* role = selRole->getText();
        bool remember = chkRemember->isChecked();

        // Wyświetl podsumowanie
        std::string summary;
        summary += "Imię: ";
        summary += (name ? name : "(puste)");
        summary += "\r\nRola: ";
        summary += (role ? role : "(brak)");
        summary += "\r\nZapamiętaj: ";
        summary += (remember ? "TAK" : "NIE");
        summary += "\r\n";

        txtResult->setText(StringUtils::utf8ToWide(summary).c_str());

        // Zapis do konfiguracji jeśli zaznaczony checkbox
        if (remember) {
            config.setValue("name", name ? name : "");
            config.setValue("role", role ? role : "");
        }
    }));

    window->add(new Button(SX + 160, y, 150, 32, "Wyczyść", [](Button*) {
        inputName->setText("");
        inputPassword->setText("");
        txtResult->setText(L"");
    }));
    y += 45;

    // --- Wynik ---
    window->add(new Label(LX, y, LW, 22, L"Wynik:"));
    txtResult = new TextArea(SX, y, SW, 120, nullptr);
    window->add(txtResult);

    // --- Odczytaj zapamiętane wartości ---
    std::string savedName = config.getValue("name", "");
    std::string savedRole = config.getValue("role", "");
    if (!savedName.empty()) {
        inputName->setText(savedName.c_str());
    }
}

void loop() {}
```

## Kluczowe punkty

1. **InputField** — `setPlaceholder()` ustawia tekst podpowiedzi, `setPasswordMode(true)` ukrywa znaki
2. **InputField::setMaxLength(n)** — limit znaków
3. **CheckBox** — konstruktor: `(x, y, w, h, text, defaultState, callback)`
4. **CheckBox::isChecked()** — odczyt aktualnego stanu
5. **Select::addItem()** — dodawanie elementów po utworzeniu
6. **ConfigManager** — `setValue()` / `getValue()` do trwałego zapisu ustawień
7. **TextArea** — readonly, idealny do wyświetlania wyników
