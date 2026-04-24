# ModbusSerialPort

`IO/Modbus/ModbusSerialPort.h` — thin synchronous serial port for **Modbus RTU** with full DCB control (parity, stop bits, data bits, hard timeout). Lives in the `modbus::` namespace.

> Different from the library's general-purpose [`Serial`](Serial.md): `ModbusSerialPort` is synchronous, blocking, fully configurable per byte (8N1 is **not** hardcoded), and tuned for request/response Modbus framing. Use `Serial` for streaming text/log device, `ModbusSerialPort` for industrial protocol stacks.

## Types

```cpp
namespace modbus {

enum class Parity   : uint8_t { None, Odd, Even };
enum class StopBits : uint8_t { One, Two };

struct SerialConfig {
    std::string  port            = "COM1";
    DWORD        baud            = 9600;
    uint8_t      dataBits        = 8;
    Parity       parity          = Parity::None;
    StopBits     stopBits        = StopBits::One;
    DWORD        readTimeoutMs   = 1000;
};

class ModbusSerialPort {
public:
    bool open (const SerialConfig& cfg, std::wstring* err = nullptr);
    void close();
    bool isOpen() const;

    // Synchronous, throws no exceptions.
    bool write (const uint8_t* data, size_t len);
    int  read  (uint8_t* buf, size_t maxLen);          // blocks up to readTimeoutMs

    static std::vector<std::string> enumPorts();        // QueryDosDeviceA
};

} // namespace modbus
```

## Example

```cpp
#include <IO/Modbus/ModbusSerialPort.h>

modbus::SerialConfig cfg;
cfg.port            = "COM5";
cfg.baud            = 19200;
cfg.parity          = modbus::Parity::Even;
cfg.stopBits        = modbus::StopBits::One;
cfg.readTimeoutMs   = 500;

modbus::ModbusSerialPort port;
std::wstring err;
if (!port.open(cfg, &err)) {
    MessageBoxW(NULL, err.c_str(), L"Port error", MB_OK);
    return;
}

uint8_t req[8] = { 0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B };
port.write(req, sizeof(req));

uint8_t resp[256];
int n = port.read(resp, sizeof(resp));   // blocks ≤ 500 ms

// List ports for a Select dropdown:
for (auto& name : modbus::ModbusSerialPort::enumPorts()) {
    select->addItem(name.c_str());
}
```

## Notes

- Win32 `CreateFileA` + DCB. No background thread.
- `read()` blocks for up to `readTimeoutMs` and returns the number of bytes received (0 on timeout, -1 on error).
- Pair with [`RtuMaster`](ModbusRTU.md) for full Modbus RTU client functionality.
