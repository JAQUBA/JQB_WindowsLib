# Serial — COM Port Communication

> `#include <IO/Serial/Serial.h>`

## Description

Serial port (COM / RS-232) communication module with:
- Automatic COM port detection (via SetupAPI)
- Threaded data reception (non-blocking)
- Callbacks: `onConnect`, `onDisconnect`, `onReceive`
- Auto-reconnect on communication errors

## Constructor

```cpp
Serial();
```

No parameters required. Connection parameters are configured internally.

## Default Connection Parameters

| Parameter | Value |
|-----------|-------|
| Baud Rate | 9600 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| DTR | Enabled |
| RTS | Enabled |

## Methods

### Initialization and Connection

| Method | Returns | Description |
|--------|---------|-------------|
| `init()` | `bool` | Initializes module — scans available COM ports |
| `connect()` | `bool` | Opens port and starts read thread |
| `disconnect()` | `void` | Closes port and stops thread |
| `isConnected()` | `bool` | Whether connection is active |
| `setPort(const char* portName)` | `void` | Sets port (e.g. `"COM3"`) |
| `updateComPorts()` | `void` | Re-scans ports |
| `getAvailablePorts()` | `const vector<string>&` | List of available ports |

### Sending and Receiving

| Method | Returns | Description |
|--------|---------|-------------|
| `write(const vector<uint8_t>& data)` | `bool` | Sends data |
| `send(const vector<uint8_t>& data)` | `bool` | Alias for `write()` |
| `read(vector<uint8_t>& data, size_t n)` | `bool` | Reads `n` bytes (synchronous) |

### Callbacks

| Method | Callback | Description |
|--------|----------|-------------|
| `onConnect(function<void()>)` | Connected | Called after successful `connect()` |
| `onDisconnect(function<void()>)` | Disconnected | Called on `disconnect()` |
| `onReceive(function<void(const vector<uint8_t>&)>)` | Data received | Called from read thread |

## Examples

### Basic Communication

```cpp
#include <IO/Serial/Serial.h>

Serial serial;

void setup() {
    serial.init();
    
    // List ports
    for (const auto& port : serial.getAvailablePorts()) {
        // e.g. "COM3", "COM5"
    }
    
    serial.setPort("COM3");
    serial.connect();
}
```

### With Callbacks

```cpp
Serial serial;

void setup() {
    serial.init();
    
    serial.onConnect([]() {
        lblStatus->setText(L"Connected!");
    });
    
    serial.onDisconnect([]() {
        lblStatus->setText(L"Disconnected");
    });
    
    serial.onReceive([](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        textLog->append(text);
    });
    
    serial.setPort("COM3");
    serial.connect();
}
```

### Integration with Select

```cpp
Serial serial;
serial.init();

Select* selPort = new Select(20, 50, 200, 25, "Port",
    [](Select* s) {
        serial.setPort(s->getText());
    }
);

// Use link() for automatic synchronization
selPort->link(&serial.getAvailablePorts());
window->add(selPort);

// To refresh the port list:
serial.updateComPorts();
selPort->updateItems();
```

### Sending Data

```cpp
std::vector<uint8_t> packet = {0x55, 0xAA, 0x01, 0x00};
serial.write(packet);

// Or using send()
serial.send(packet);
```

## Read Thread

The read thread runs in the background after `connect()`:
1. Every 10 ms checks `ClearCommError()` for available data
2. If data available → `ReadFile()` and invokes `onReceive` callback
3. After several consecutive errors → auto-reconnect (disconnect + connect)
4. Stopped on `disconnect()` or destructor

## Notes

- **Port enumeration:** Uses `SetupDiGetClassDevsA` with `GUID_DEVCLASS_PORTS`. Gets "Friendly Name" (e.g. `"USB Serial Port (COM3)"`) and extracts the port number
- Port names: `"COM1"`, `"COM3"` etc. — without `\\.\` (prefix added internally)
- **Timeouts:** ReadInterval=50, ReadTotal=50+10×bytes, WriteTotal=50+10×bytes
- Read buffer: up to 256 bytes per cycle
- `maxConsecutiveErrors = 10` → auto-reconnect after 10 consecutive errors
