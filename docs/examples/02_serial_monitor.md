# Przykład 02 — Serial Monitor

Monitor portu szeregowego z wyborem portu, przyciskami połączenia i logiem komunikacji.

## `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
build_flags =
    -std=c++17
```

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/Select/Select.h>
#include <UI/TextArea/TextArea.h>
#include <UI/InputField/InputField.h>
#include <IO/Serial/Serial.h>
#include <Util/StringUtils.h>

SimpleWindow* window;
Label*       lblStatus;
Select*      selPort;
Button*      btnConnect;
TextArea*    txtLog;
InputField*  inputSend;
Serial       serial;

// Wektor portów COM do powiązania z Select
std::vector<std::string> portList;

void refreshPorts() {
    serial.init();  // skanuje porty
    portList.clear();
    // Serial::init() wypełnia wewnętrzną listę — tu ręcznie dodajemy znane porty
    // W praktyce: użyj serial.getAvailablePorts() jeśli API to wspiera,
    // lub ręcznie enumeruj z SetupAPI
    portList.push_back("COM1");
    portList.push_back("COM3");
    portList.push_back("COM4");
    selPort->updateItems(portList);
}

void appendLog(const char* prefix, const std::vector<uint8_t>& data) {
    std::string line = prefix;
    char hex[4];
    for (uint8_t b : data) {
        snprintf(hex, sizeof(hex), "%02X ", b);
        line += hex;
    }
    line += "\r\n";
    std::wstring wline = StringUtils::utf8ToWide(line);
    txtLog->append(wline.c_str());
}

void setup() {
    window = new SimpleWindow(700, 500, "Serial Monitor", 0);
    window->init();

    int y = 10;

    // --- Pasek statusu ---
    lblStatus = new Label(10, y, 300, 22, L"Status: Rozłączony");
    window->add(lblStatus);
    y += 30;

    // --- Wybór portu ---
    window->add(new Label(10, y, 60, 22, L"Port:"));
    selPort = new Select(75, y, 120, 200, "COM1", nullptr);
    window->add(selPort);

    btnConnect = new Button(210, y, 100, 25, "Połącz", [](Button* btn) {
        if (serial.isConnected()) {
            serial.disconnect();
            lblStatus->setText(L"Status: Rozłączony");
            SetWindowTextA(btnConnect->getHandle(), "Połącz");
        } else {
            const char* port = selPort->getText();
            if (port && port[0]) {
                serial.setPort(port);
                serial.connect();
            }
        }
    });
    window->add(btnConnect);

    window->add(new Button(320, y, 100, 25, "Odśwież", [](Button*) {
        refreshPorts();
    }));
    y += 35;

    // --- Log ---
    txtLog = new TextArea(10, y, 670, 320, nullptr);
    window->add(txtLog);
    y += 330;

    // --- Pole wysyłania ---
    inputSend = new InputField(10, y, 560, 28, "", nullptr);
    window->add(inputSend);

    window->add(new Button(580, y, 90, 28, "Wyślij", [](Button*) {
        if (!serial.isConnected()) {
            lblStatus->setText(L"Najpierw połącz!");
            return;
        }
        const char* text = inputSend->getText();
        if (text && text[0]) {
            std::vector<uint8_t> data(text, text + strlen(text));
            serial.write(data);

            std::string line = "> TX: ";
            line += text;
            line += "\r\n";
            txtLog->append(StringUtils::utf8ToWide(line).c_str());
        }
    }));

    // --- Callbacki Serial ---
    serial.onConnect([]() {
        lblStatus->setText(L"Status: Połączony");
        SetWindowTextA(btnConnect->getHandle(), "Rozłącz");
        txtLog->append(L"--- Połączono ---\r\n");
    });

    serial.onDisconnect([]() {
        lblStatus->setText(L"Status: Rozłączony");
        SetWindowTextA(btnConnect->getHandle(), "Połącz");
        txtLog->append(L"--- Rozłączono ---\r\n");
    });

    serial.onReceive([](const std::vector<uint8_t>& data) {
        appendLog("< RX: ", data);
    });

    serial.init();
    refreshPorts();
}

void loop() {
    // Serial działa na osobnym wątku — tu nic nie potrzeba
}
```

## Kluczowe punkty

1. **Serial** — komunikacja COM port, wątkowy odbiór danych
2. `serial.init()` — musi być wywołane przed `connect()`
3. `serial.onReceive()` — callback wywoływany z wątku odbioru (uwaga na thread-safety!)
4. **TextArea** — readonly, `append()` dodaje tekst z auto-scroll
5. **InputField** — pole edycyjne, `getText()` zwraca aktualny tekst
6. **Select** — combo box, `getText()` zwraca wybrany element
7. **StringUtils::utf8ToWide()** — konwersja UTF-8 → UTF-16 do metod `wchar_t*`
