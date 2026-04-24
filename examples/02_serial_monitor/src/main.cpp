#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/Select/Select.h>
#include <UI/TextArea/TextArea.h>
#include <UI/InputField/InputField.h>
#include <IO/Serial/Serial.h>
#include <Util/StringUtils.h>

#include <vector>
#include <string>

SimpleWindow* g_window = nullptr;
Label* g_status = nullptr;
Select* g_portsSelect = nullptr;
TextArea* g_log = nullptr;
InputField* g_txInput = nullptr;
Button* g_connectBtn = nullptr;

Serial g_serial;
std::vector<std::string> g_ports;

void appendLog(const std::wstring& msg) {
    if (!g_log) return;
    g_log->append(msg + L"\r\n");
}

void setConnectionUi(bool connected) {
    if (g_status) {
        g_status->setText(connected ? L"Status: Connected" : L"Status: Disconnected");
    }
    if (g_connectBtn && g_connectBtn->getHandle()) {
        SetWindowTextA(g_connectBtn->getHandle(), connected ? "Disconnect" : "Connect");
    }
}

void refreshPorts() {
    g_serial.updateComPorts();
    g_ports = g_serial.getAvailablePorts();

    if (g_ports.empty()) {
        g_ports.push_back("(no ports)");
    }

    if (g_portsSelect) {
        g_portsSelect->updateItems();
    }
}

void setup() {
    g_window = new SimpleWindow(760, 560, "Example 02 - Serial Monitor", 0);
    g_window->init();

    g_status = new Label(12, 10, 280, 22, L"Status: Disconnected");
    g_window->add(g_status);

    g_portsSelect = new Select(12, 38, 220, 120, "(no ports)", nullptr);
    g_portsSelect->link(&g_ports);
    g_window->add(g_portsSelect);

    g_connectBtn = new Button(242, 38, 120, 28, "Connect", [](Button*) {
        if (g_serial.isConnected()) {
            g_serial.disconnect();
            setConnectionUi(false);
            appendLog(L"Disconnected");
            return;
        }

        const char* sel = g_portsSelect ? g_portsSelect->getText() : "";
        if (!sel || !sel[0] || std::string(sel) == "(no ports)") {
            appendLog(L"No valid COM port selected");
            return;
        }

        g_serial.setPort(sel);
        if (!g_serial.connect()) {
            appendLog(L"Connect failed");
            return;
        }
    });
    g_window->add(g_connectBtn);

    g_window->add(new Button(372, 38, 120, 28, "Refresh", [](Button*) {
        refreshPorts();
    }));

    g_log = new TextArea(12, 78, 730, 400);
    g_window->add(g_log);

    g_txInput = new InputField(12, 490, 600, 30, "", nullptr);
    g_window->add(g_txInput);

    g_window->add(new Button(622, 490, 120, 30, "Send", [](Button*) {
        if (!g_serial.isConnected()) {
            appendLog(L"Cannot send: not connected");
            return;
        }

        std::string txt = g_txInput ? g_txInput->getText() : "";
        if (txt.empty()) return;

        std::vector<uint8_t> payload(txt.begin(), txt.end());
        if (g_serial.write(payload)) {
            std::wstring line = L"TX: ";
            line += StringUtils::utf8ToWide(txt);
            appendLog(line);
        }
    }));

    g_serial.onConnect([]() {
        setConnectionUi(true);
        appendLog(L"Connected");
    });

    g_serial.onDisconnect([]() {
        setConnectionUi(false);
        appendLog(L"Disconnected callback");
    });

    g_serial.onReceive([](const std::vector<uint8_t>& data) {
        std::wstring line = L"RX: ";
        for (uint8_t b : data) {
            wchar_t hex[8];
            _snwprintf(hex, 8, L"%02X ", b);
            line += hex;
        }
        appendLog(line);
    });

    g_serial.onError([]() {
        appendLog(L"Serial error callback");
        setConnectionUi(false);
    });

    g_serial.init();
    refreshPorts();
}

void loop() {
    if (g_serial.isConnectionLost()) {
        g_serial.clearConnectionLost();
        setConnectionUi(false);
        appendLog(L"Connection lost");
    }
}
