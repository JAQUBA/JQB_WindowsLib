# Example 02 — Serial Monitor

Serial port monitor with port selection, connect/disconnect buttons, and communication log.

## `platformio.ini`

```ini
[env:app]
platform = native
lib_deps =
    https://github.com/JAQUBA/JQB_WindowsLib.git
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

// COM port list linked to Select
std::vector<std::string> portList;

void refreshPorts() {
    serial.init();  // scans ports
    portList.clear();
    // Serial::init() populates the internal list — here we manually add known ports
    // In practice: use serial.getAvailablePorts() if the API supports it,
    // or manually enumerate via SetupAPI
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

    // --- Status bar ---
    lblStatus = new Label(10, y, 300, 22, L"Status: Disconnected");
    window->add(lblStatus);
    y += 30;

    // --- Port selection ---
    window->add(new Label(10, y, 60, 22, L"Port:"));
    selPort = new Select(75, y, 120, 200, "COM1", nullptr);
    window->add(selPort);

    btnConnect = new Button(210, y, 100, 25, "Connect", [](Button* btn) {
        if (serial.isConnected()) {
            serial.disconnect();
            lblStatus->setText(L"Status: Disconnected");
            SetWindowTextA(btnConnect->getHandle(), "Connect");
        } else {
            const char* port = selPort->getText();
            if (port && port[0]) {
                serial.setPort(port);
                serial.connect();
            }
        }
    });
    window->add(btnConnect);

    window->add(new Button(320, y, 100, 25, "Refresh", [](Button*) {
        refreshPorts();
    }));
    y += 35;

    // --- Log ---
    txtLog = new TextArea(10, y, 670, 320, nullptr);
    window->add(txtLog);
    y += 330;

    // --- Send field ---
    inputSend = new InputField(10, y, 560, 28, "", nullptr);
    window->add(inputSend);

    window->add(new Button(580, y, 90, 28, "Send", [](Button*) {
        if (!serial.isConnected()) {
            lblStatus->setText(L"Connect first!");
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

    // --- Serial callbacks ---
    serial.onConnect([]() {
        lblStatus->setText(L"Status: Connected");
        SetWindowTextA(btnConnect->getHandle(), "Disconnect");
        txtLog->append(L"--- Connected ---\r\n");
    });

    serial.onDisconnect([]() {
        lblStatus->setText(L"Status: Disconnected");
        SetWindowTextA(btnConnect->getHandle(), "Connect");
        txtLog->append(L"--- Disconnected ---\r\n");
    });

    serial.onReceive([](const std::vector<uint8_t>& data) {
        appendLog("< RX: ", data);
    });

    serial.init();
    refreshPorts();
}

void loop() {
    // Serial runs on a separate thread — nothing needed here
}
```

## Key Points

1. **Serial** — COM port communication with threaded data reception
2. `serial.init()` — must be called before `connect()`
3. `serial.onReceive()` — callback invoked from the receive thread (beware of thread safety!)
4. **TextArea** — readonly, `append()` adds text with auto-scroll
5. **InputField** — editable field, `getText()` returns the current text
6. **Select** — combo box, `getText()` returns the selected item
7. **StringUtils::utf8ToWide()** — UTF-8 → UTF-16 conversion for `wchar_t*` methods
