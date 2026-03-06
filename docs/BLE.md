# BLE — Bluetooth Low Energy

> `#include <IO/BLE/BLE.h>`

## Description

Universal Bluetooth Low Energy (BLE) communication module with:
- BT adapter availability check
- BLE device scanning (via SetupAPI) with optional filter
- Device prioritization in results list
- Device connection
- **GATT support** — automatic service discovery, notification registration, characteristic write
- Callbacks: connect, disconnect, receive, deviceDiscovered, scanComplete, error

## Connection State

```cpp
enum class ConnectionState {
    DISCONNECTED,       // Disconnected
    SCANNING,           // Scanning
    CONNECTING,         // Connecting
    CONNECTED,          // Connected
    CONNECTION_ERROR    // Connection error
};
```

## `BLEDevice` Structure

```cpp
struct BLEDevice {
    std::wstring name;         // Device name
    std::wstring address;      // Address (device path)
    int rssi;                  // Signal strength (0 if unavailable)
    bool isConnectable;        // Whether connection is possible
    
    std::string toString() const;  // "DeviceName (RSSI: -50 dBm)"
};
```

## `BLEScanFilter` Structure

Device filter applied during scanning. Both fields are optional (case-insensitive).

```cpp
struct BLEScanFilter {
    std::wstring nameContains;   // Device name fragment
    std::wstring pathContains;   // Device path fragment
    
    BLEScanFilter();                              // No filter
    BLEScanFilter(const std::wstring& name);      // Filter by name
    BLEScanFilter(const std::wstring& name,
                  const std::wstring& path);      // Filter by name and path
    bool isEmpty() const;
};
```

## Methods

### Initialization

| Method | Returns | Description |
|--------|---------|-------------|
| `init()` | `bool` | Initialization — checks BT adapter |
| `isAvailable()` | `bool` | Whether BLE is available in the system |

### Scanning

| Method | Returns | Description |
|--------|---------|-------------|
| `startScan(int seconds = 10)` | `bool` | Scan for all BLE devices |
| `startScan(const BLEScanFilter& filter, int seconds = 10)` | `bool` | Scan with filter |
| `stopScan()` | `void` | Stops scanning |
| `isScanning()` | `bool` | Whether scanning is in progress |
| `getDiscoveredDevices()` | `const vector<BLEDevice>&` | List of found devices |
| `getAvailableDevices()` | `const vector<string>&` | Device list as strings (for Select) |

### Connection

| Method | Returns | Description |
|--------|---------|-------------|
| `connect(const wstring& address)` | `bool` | Connects to device (in thread) |
| `disconnect()` | `void` | Disconnects |
| `isConnected()` | `bool` | Whether connected |
| `setDevice(const wstring& name)` | `void` | Sets device by name/address |
| `getConnectionState()` | `ConnectionState` | Current state |

### GATT Configuration (before connect)

| Method | Description |
|--------|-------------|
| `setServiceUUID(const wstring& uuid)` | GATT service UUID (e.g. `L"0000fff0-0000-1000-8000-00805f9b34fb"`) |
| `setNotifyCharacteristicUUID(const wstring& uuid)` | Notify characteristic UUID (data reception). Optional — auto-detect if omitted |
| `setWriteCharacteristicUUID(const wstring& uuid)` | Write characteristic UUID (sending data). Optional — auto-detect if omitted |

> **Note:** `setServiceUUID()` is required to activate GATT. If you don't set the service UUID, the connection will be established without GATT (raw handle).

### Sending Data

| Method | Returns | Description |
|--------|---------|-------------|
| `write(const vector<uint8_t>& data)` | `bool` | Sends data via GATT characteristic write (or overlapped I/O as fallback) |
| `send(const vector<uint8_t>& data)` | `bool` | Alias for `write()` |

### Device Prioritization

| Method | Description |
|--------|-------------|
| `addPriorityFilter(const wstring& fragment)` | Devices matching the name/path fragment appear at the top of the list |
| `clearPriorityFilters()` | Removes all priority filters |

### Callbacks

| Method | Parameters | When |
|--------|------------|------|
| `onConnect(function<void()>)` | — | After connection established |
| `onDisconnect(function<void()>)` | — | After disconnection |
| `onReceive(function<void(const vector<uint8_t>&)>)` | data | Data received |
| `onDeviceDiscovered(function<void(const BLEDevice&)>)` | device | New device found |
| `onScanComplete(function<void()>)` | — | Scanning completed |
| `onError(function<void(const wstring&)>)` | message | Error occurred |

## Examples

### Scanning All BLE Devices

```cpp
#include <IO/BLE/BLE.h>

BLE ble;

void setup() {
    if (!ble.init()) return;

    // GATT configuration (service and characteristic UUIDs)
    ble.setServiceUUID(L"0000fff0-0000-1000-8000-00805f9b34fb");
    ble.setNotifyCharacteristicUUID(L"0000fff4-0000-1000-8000-00805f9b34fb");
    ble.setWriteCharacteristicUUID(L"0000fff3-0000-1000-8000-00805f9b34fb");

    ble.onDeviceDiscovered([](const BLEDevice& device) {
        // New device found
    });

    ble.onScanComplete([]() {
        auto& devices = ble.getDiscoveredDevices();
        if (!devices.empty()) {
            ble.connect(devices[0].address);
        }
    });

    ble.onConnect([]() { /* Connected — GATT configured automatically */ });
    ble.onReceive([](const std::vector<uint8_t>& data) { /* Data from GATT notify */ });
    ble.onError([](const std::wstring& error) { /* Error */ });

    ble.startScan(10);
}
```

### Scanning with Name Filter

```cpp
// Scan only devices containing "OW18B" in name
ble.startScan(BLEScanFilter(L"OW18B"), 10);
```

### Scanning with Name and Path Filter

```cpp
// Filter by name and path fragment
ble.startScan(BLEScanFilter(L"", L"dev_aabbccddee"), 10);
```

### Device Prioritization

```cpp
// OWON devices appear at the top of the list
ble.addPriorityFilter(L"owon");
ble.addPriorityFilter(L"ow18b");
ble.startScan(10);
```

### Integration with Select

```cpp
BLE ble;
ble.init();

Select* selDevice = new Select(20, 50, 300, 25, "BLE Device",
    [](Select* s) {
        int idx = /* get index */;
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

### Sending Data

```cpp
std::vector<uint8_t> cmd = {0x01, 0x02, 0x03};
ble.write(cmd);
```

## Threads

| Thread | Function | Description |
|--------|----------|-------------|
| Scanning | `scanThreadFunction()` | BLE device enumeration via SetupAPI |
| Connection | `connectionThreadFunction()` | `CreateFileW()` to BLE device + GATT setup |

> GATT notifications don't require a separate thread — the callback is invoked by the Windows thread pool and marshaled to the UI thread via PostMessage.

## Notes

- Implementation uses **Windows SetupAPI** for enumeration and **Windows GATT API** (`BluetoothApis.dll`) for communication
- `BluetoothApis.dll` loaded dynamically in `init()` — no static linking
- `startScan()` enumerates paired BLE devices (not active BLE advertising)
- Callbacks from worker threads are marshaled to the UI thread via PostMessage
- The class contains no constants specific to any particular device — UUIDs and filters are configured by the application
- If `setServiceUUID()` is set → `connect()` automatically performs GATT discovery, CCCD enable, and notification registration
- `write()` uses `BluetoothGATTSetCharacteristicValue` when GATT is available, with fallback to overlapped `WriteFile`
