# Przykład 06 — BLE Scanner

Skaner urządzeń Bluetooth Low Energy z listą znalezionych urządzeń i logiem.

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

    lblStatus = new Label(10, y, 400, 22, L"Status: Gotowy");
    window->add(lblStatus);
    y += 30;

    // --- Pasek skanowania ---
    progressScan = new ProgressBar(10, y, 620, 20);
    window->add(progressScan);
    y += 30;

    // --- Przyciski ---
    btnScan = new Button(10, y, 130, 30, "Skanuj (10s)", [](Button*) {
        lblStatus->setText(L"Status: Skanowanie...");
        progressScan->setMarquee(true);
        deviceNames.clear();
        selDevices->updateItems(deviceNames);
        txtLog->setText(L"");
        logMsg(L"Rozpoczynam skanowanie BLE...");
        ble.startScan(10);
    });
    window->add(btnScan);

    btnConnect = new Button(150, y, 130, 30, "Połącz", [](Button*) {
        auto& devices = ble.getDiscoveredDevices();
        int idx = (int)SendMessage(selDevices->getHandle(), CB_GETCURSEL, 0, 0);
        if (idx >= 0 && idx < (int)devices.size()) {
            logMsg(L"Łączenie...");
            ble.connect(devices[idx].address);
        } else {
            logMsg(L"Wybierz urządzenie z listy!");
        }
    });
    window->add(btnConnect);

    window->add(new Button(290, y, 130, 30, "Rozłącz", [](Button*) {
        ble.disconnect();
    }));
    y += 40;

    // --- Lista urządzeń ---
    window->add(new Label(10, y, 150, 22, L"Znalezione:"));
    selDevices = new Select(160, y, 470, 200, "(brak)", nullptr);
    window->add(selDevices);
    y += 35;

    // --- Log ---
    txtLog = new TextArea(10, y, 620, 250, nullptr);
    window->add(txtLog);

    // --- Callbacki BLE ---
    ble.onDeviceDiscovered([](const BLEDevice& dev) {
        std::wstring msg = L"Znaleziono: " + dev.name +
            L" [" + dev.address + L"]";
        logMsg(msg.c_str());

        // Dodaj do selecta
        std::string nameUtf8 = StringUtils::wideToUtf8(dev.name);
        deviceNames.push_back(nameUtf8);
        selDevices->updateItems(deviceNames);
    });

    ble.onScanComplete([]() {
        progressScan->setMarquee(false);
        progressScan->setProgress(100);
        lblStatus->setText(L"Status: Skanowanie zakończone");
        
        auto& devices = ble.getDiscoveredDevices();
        wchar_t buf[64];
        swprintf(buf, 64, L"Znaleziono %d urządzeń", (int)devices.size());
        logMsg(buf);
    });

    ble.onConnect([]() {
        lblStatus->setText(L"Status: Połączony");
        logMsg(L"--- Połączono z urządzeniem BLE ---");
    });

    ble.onDisconnect([]() {
        lblStatus->setText(L"Status: Rozłączony");
        logMsg(L"--- Rozłączono ---");
    });

    ble.onReceive([](const std::vector<uint8_t>& data) {
        std::wstring msg = L"RX [" + std::to_wstring(data.size()) + L" B]: ";
        for (uint8_t b : data) {
            wchar_t hex[4];
            swprintf(hex, 4, L"%02X ", b);
            msg += hex;
        }
        logMsg(msg.c_str());
    });

    ble.onError([](const std::wstring& errMsg) {
        std::wstring msg = L"BŁĄD: " + errMsg;
        logMsg(msg.c_str());
        lblStatus->setText(L"Status: Błąd");
        progressScan->setMarquee(false);
    });

    // --- Inicjalizacja ---
    ble.init();
    logMsg(L"Adapter BLE zainicjalizowany.");
}

void loop() {}
```

## Kluczowe punkty

1. **BLE** — komunikacja Bluetooth Low Energy
   - `init()` — sprawdza adapter BT, musi być wywołane przed innymi operacjami
   - `startScan(seconds)` — skanuje przez podany czas
   - `connect(address)` — łączy się z urządzeniem po adresie
   - `disconnect()` — rozłącza

2. **Callbacki BLE**:
   - `onDeviceDiscovered()` — wywoływany dla każdego znalezionego urządzenia
   - `onScanComplete()` — skanowanie zakończone
   - `onConnect()` / `onDisconnect()` — zmiana stanu połączenia
   - `onReceive()` — odebrano dane
   - `onError()` — błąd (np. brak adaptera)

3. **BLEDevice** struct:
   - `name` — `std::wstring` (nazwa urządzenia)
   - `address` — `std::wstring` (adres MAC)

4. **ProgressBar::setMarquee(true)** — animacja ciągła (bez wartości %)
