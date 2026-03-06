# Example 06 — BLE Scanner

Bluetooth Low Energy device scanner with a discovered device list and log.

## `src/main.cpp`

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>
#include <UI/Button/Button.h>
#include <UI/Select/Select.h>
#include <UI/TextArea/TextArea.h>
#include <UI/ProgressBar/ProgressBar.h>
#include <IO/BLE/BLE.h>
#include <Util/StringUtils.h>

SimpleWindow*   window;
Label*          lblStatus;
Select*         selDevices;
TextArea*       txtLog;
ProgressBar*    progressScan;
Button*         btnScan;
Button*         btnConnect;
BLE             ble;

std::vector<std::string> deviceNames;

void logMsg(const wchar_t* msg) {
    txtLog->append(msg);
    txtLog->append(L"\r\n");
}

void setup() {
    window = new SimpleWindow(650, 480, "BLE Scanner", 0);
    window->init();

    int y = 10;

    lblStatus = new Label(10, y, 400, 22, L"Status: Ready");
    window->add(lblStatus);
    y += 30;

    // --- Scan progress bar ---
    progressScan = new ProgressBar(10, y, 620, 20);
    window->add(progressScan);
    y += 30;

    // --- Buttons ---
    btnScan = new Button(10, y, 130, 30, "Scan (10s)", [](Button*) {
        lblStatus->setText(L"Status: Scanning...");
        progressScan->setMarquee(true);
        deviceNames.clear();
        selDevices->updateItems(deviceNames);
        txtLog->setText(L"");
        logMsg(L"Starting BLE scan...");
        ble.startScan(10);
    });
    window->add(btnScan);

    btnConnect = new Button(150, y, 130, 30, "Connect", [](Button*) {
        auto& devices = ble.getDiscoveredDevices();
        int idx = (int)SendMessage(selDevices->getHandle(), CB_GETCURSEL, 0, 0);
        if (idx >= 0 && idx < (int)devices.size()) {
            logMsg(L"Connecting...");
            ble.connect(devices[idx].address);
        } else {
            logMsg(L"Select a device from the list!");
        }
    });
    window->add(btnConnect);

    window->add(new Button(290, y, 130, 30, "Disconnect", [](Button*) {
        ble.disconnect();
    }));
    y += 40;

    // --- Device list ---
    window->add(new Label(10, y, 150, 22, L"Discovered:"));
    selDevices = new Select(160, y, 470, 200, "(none)", nullptr);
    window->add(selDevices);
    y += 35;

    // --- Log ---
    txtLog = new TextArea(10, y, 620, 250, nullptr);
    window->add(txtLog);

    // --- BLE callbacks ---
    ble.onDeviceDiscovered([](const BLEDevice& dev) {
        std::wstring msg = L"Found: " + dev.name +
            L" [" + dev.address + L"]";
        logMsg(msg.c_str());

        // Add to select
        std::string nameUtf8 = StringUtils::wideToUtf8(dev.name);
        deviceNames.push_back(nameUtf8);
        selDevices->updateItems(deviceNames);
    });

    ble.onScanComplete([]() {
        progressScan->setMarquee(false);
        progressScan->setProgress(100);
        lblStatus->setText(L"Status: Scan complete");
        
        auto& devices = ble.getDiscoveredDevices();
        wchar_t buf[64];
        swprintf(buf, 64, L"Found %d devices", (int)devices.size());
        logMsg(buf);
    });

    ble.onConnect([]() {
        lblStatus->setText(L"Status: Connected");
        logMsg(L"--- Connected to BLE device ---");
    });

    ble.onDisconnect([]() {
        lblStatus->setText(L"Status: Disconnected");
        logMsg(L"--- Disconnected ---");
    });

    ble.onReceive([](const std::vector<uint8_t>& data) {
        std::wstring msg = L"RX [" + jqb_compat::to_wstring(data.size()) + L" B]: ";
        for (uint8_t b : data) {
            wchar_t hex[4];
            swprintf(hex, 4, L"%02X ", b);
            msg += hex;
        }
        logMsg(msg.c_str());
    });

    ble.onError([](const std::wstring& errMsg) {
        std::wstring msg = L"ERROR: " + errMsg;
        logMsg(msg.c_str());
        lblStatus->setText(L"Status: Error");
        progressScan->setMarquee(false);
    });

    // --- Init ---
    ble.init();
    logMsg(L"BLE adapter initialized.");
}

void loop() {}
```

## Key Points

1. **BLE** — Bluetooth Low Energy communication
   - `init()` — checks the BT adapter, must be called before other operations
   - `startScan(seconds)` — scans for the given duration
   - `connect(address)` — connects to a device by address
   - `disconnect()` — disconnects

2. **BLE callbacks**:
   - `onDeviceDiscovered()` — called for each found device
   - `onScanComplete()` — scan finished
   - `onConnect()` / `onDisconnect()` — connection state change
   - `onReceive()` — data received
   - `onError()` — error (e.g. no adapter)

3. **BLEDevice** struct:
   - `name` — `std::wstring` (device name)
   - `address` — `std::wstring` (MAC address)

4. **ProgressBar::setMarquee(true)** — continuous animation (no % value)
