# BLE — Bluetooth Low Energy

> `#include <IO/BLE/BLE.h>`

## Opis

Moduł komunikacji Bluetooth Low Energy (BLE) z:
- Sprawdzaniem dostępności adaptera BT
- Skanowaniem urządzeń BLE (przez SetupAPI)
- Łączeniem z urządzeniami
- Wątkowym odbiorem danych (overlapped I/O)
- Callbackami: connect, disconnect, receive, deviceDiscovered, scanComplete, error

## Stan połączenia

```cpp
enum class ConnectionState {
    DISCONNECTED,       // Rozłączony
    SCANNING,           // Skanowanie
    CONNECTING,         // Łączenie
    CONNECTED,          // Połączony
    CONNECTION_ERROR    // Błąd połączenia
};
```

## Struktura `BLEDevice`

```cpp
struct BLEDevice {
    std::wstring name;         // Nazwa urządzenia
    std::wstring address;      // Adres (device path)
    int rssi;                  // Siła sygnału (0 jeśli niedostępne)
    bool isConnectable;        // Czy można się połączyć
    
    std::string toString() const;  // "NazwaUrzadzenia (RSSI: -50 dBm)"
};
```

## Metody

### Inicjalizacja

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `init()` | `bool` | Inicjalizacja — sprawdza adapter BT |
| `isAvailable()` | `bool` | Czy BLE jest dostępne w systemie |

### Skanowanie

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `startScan(int seconds = 10)` | `bool` | Rozpoczyna skanowanie (w wątku) |
| `stopScan()` | `void` | Zatrzymuje skanowanie |
| `isScanning()` | `bool` | Czy skanowanie trwa |
| `getDiscoveredDevices()` | `const vector<BLEDevice>&` | Lista znalezionych urządzeń |
| `getAvailableDevices()` | `const vector<string>&` | Lista urządzeń jako stringi (dla Select) |

### Połączenie

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `connect(const wstring& address)` | `bool` | Łączy się z urządzeniem (w wątku) |
| `disconnect()` | `void` | Rozłącza |
| `isConnected()` | `bool` | Czy połączony |
| `setDevice(const wstring& name)` | `void` | Ustawia urządzenie po nazwie/adresie |
| `getConnectionState()` | `ConnectionState` | Aktualny stan |
| `getConnectionStateString()` | `wstring` | Stan jako tekst (PL) |

### Wysyłanie

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `write(const vector<uint8_t>& data)` | `bool` | Wysyła dane (overlapped I/O) |
| `send(const vector<uint8_t>& data)` | `bool` | Alias dla `write()` |

### Callbacki

| Metoda | Parametry | Kiedy |
|--------|-----------|-------|
| `onConnect(function<void()>)` | — | Po nawiązaniu połączenia |
| `onDisconnect(function<void()>)` | — | Po rozłączeniu |
| `onReceive(function<void(const vector<uint8_t>&)>)` | dane | Odebrano dane |
| `onDeviceDiscovered(function<void(const BLEDevice&)>)` | urządzenie | Znaleziono nowe urządzenie |
| `onScanComplete(function<void()>)` | — | Zakończono skanowanie |
| `onError(function<void(const wstring&)>)` | komunikat | Wystąpił błąd |

## Przykłady

### Skanowanie i połączenie

```cpp
#include <IO/BLE/BLE.h>

BLE ble;

void setup() {
    if (!ble.init()) {
        // Bluetooth niedostępny
        return;
    }
    
    ble.onDeviceDiscovered([](const BLEDevice& device) {
        // Nowe urządzenie znalezione
    });
    
    ble.onScanComplete([]() {
        // Skanowanie zakończone
        auto& devices = ble.getDiscoveredDevices();
        if (!devices.empty()) {
            ble.connect(devices[0].address);
        }
    });
    
    ble.onConnect([]() {
        lblStatus->setText(L"Połączono przez BLE!");
    });
    
    ble.onReceive([](const std::vector<uint8_t>& data) {
        // Odebrano dane
    });
    
    ble.onError([](const std::wstring& error) {
        // Obsługa błędu
    });
    
    ble.startScan(10);  // Skanuj 10 sekund
}
```

### Integracja z Select

```cpp
BLE ble;
ble.init();

Select* selDevice = new Select(20, 50, 300, 25, "Urządzenie BLE",
    [](Select* s) {
        int idx = /* pobierz indeks */;
        auto& devices = ble.getDiscoveredDevices();
        if (idx >= 0 && idx < devices.size()) {
            ble.connect(devices[idx].address);
        }
    }
);
selDevice->link(&ble.getAvailableDevices());
window->add(selDevice);

ble.onScanComplete([selDevice]() {
    selDevice->updateItems();
});

ble.startScan();
```

### Wysyłanie danych

```cpp
std::vector<uint8_t> cmd = {0x01, 0x02, 0x03};
ble.write(cmd);
```

## Stałe UUID (OWON OW18B)

```cpp
static const std::wstring OWON_SERVICE_UUID;                    // 0000fff0-...
static const std::wstring OWON_NOTIFY_CHARACTERISTIC_UUID;      // 0000fff4-...
static const std::wstring OWON_WRITE_CHARACTERISTIC_UUID;       // 0000fff3-...
```

## Wątki

| Wątek | Funkcja | Opis |
|-------|---------|------|
| Skanowanie | `scanThreadFunction()` | Enumeracja urządzeń BLE przez SetupAPI |
| Połączenie | `connectionThreadFunction()` | `CreateFileW()` do urządzenia BLE |
| Powiadomienia | `notificationThreadFunction()` | Overlapped `ReadFile()` z timeout 100 ms |

## Uwagi

- Implementacja używa **Windows SetupAPI** do enumeracji i **CreateFile** do I/O
- Pełna obsługa GATT (BluetoothGATT*) wymaga Windows SDK, tu implementacja jest uproszczona
- I/O jest overlapped (`FILE_FLAG_OVERLAPPED`)
- `startScan()` enumeruje sparowane urządzenia BLE (nie aktywny BLE advertising)
- `getConnectionStateString()` zwraca tekst po polsku: „Rozłączony", „Łączenie...", „Połączony", „Błąd"
- Urządzenia OWON priorytetyzowane na liście (dodawane na początek)
