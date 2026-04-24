# ModbusRTU

`IO/Modbus/ModbusRTU.h` — synchronous **Modbus RTU master** with CRC-16 framing. Lives in the `modbus::` namespace. Pair with [`ModbusSerialPort`](ModbusSerialPort.md).

## Supported Function Codes

| FC | Constant | Method |
|---|---|---|
| 01 | `FC::ReadCoils`             | `readCoils(slave, addr, count, out)` |
| 02 | `FC::ReadDiscreteInputs`    | `readDiscreteInputs(slave, addr, count, out)` |
| 03 | `FC::ReadHoldingRegisters`  | `readHoldingRegisters(slave, addr, count, out)` |
| 04 | `FC::ReadInputRegisters`    | `readInputRegisters(slave, addr, count, out)` |
| 05 | `FC::WriteSingleCoil`       | `writeSingleCoil(slave, addr, value)` |
| 06 | `FC::WriteSingleRegister`   | `writeSingleRegister(slave, addr, value)` |
| 15 | `FC::WriteMultipleCoils`    | `writeMultipleCoils(slave, addr, values)` |
| 16 | `FC::WriteMultipleRegisters`| `writeMultipleRegisters(slave, addr, values)` |

## Result Object

```cpp
namespace modbus {

enum class Status {
    Ok,
    Timeout,
    CrcError,
    ExceptionCode,   // valid frame, but device returned exception
    ProtocolError,
    PortClosed,
    BadResponse,
};

struct Result {
    Status       status;
    uint8_t      exceptionCode;   // when status == ExceptionCode
    std::wstring message;
    bool ok() const { return status == Status::Ok; }
};

} // namespace modbus
```

## API

```cpp
class RtuMaster {
public:
    explicit RtuMaster(ModbusSerialPort& port);

    void  setTimeout(DWORD ms);
    DWORD getTimeout() const;

    // Reads
    Result readCoils            (uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>&     out);
    Result readDiscreteInputs   (uint8_t slave, uint16_t addr, uint16_t count, std::vector<bool>&     out);
    Result readHoldingRegisters (uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out);
    Result readInputRegisters   (uint8_t slave, uint16_t addr, uint16_t count, std::vector<uint16_t>& out);

    // Writes
    Result writeSingleCoil      (uint8_t slave, uint16_t addr, bool value);
    Result writeSingleRegister  (uint8_t slave, uint16_t addr, uint16_t value);
    Result writeMultipleCoils   (uint8_t slave, uint16_t addr, const std::vector<bool>&     values);
    Result writeMultipleRegisters(uint8_t slave, uint16_t addr, const std::vector<uint16_t>& values);

    // Inspect last frames (for logging / hex view)
    const std::vector<uint8_t>& lastTx() const;
    const std::vector<uint8_t>& lastRx() const;
};

// Helpers
uint16_t    crc16  (const uint8_t* data, size_t len);   // table-driven, poly 0xA001
std::wstring toHex(const std::vector<uint8_t>& bytes);  // "01 03 04 12 34 56 78 AA BB"
```

## Example

```cpp
#include <IO/Modbus/ModbusSerialPort.h>
#include <IO/Modbus/ModbusRTU.h>

modbus::ModbusSerialPort port;
modbus::SerialConfig cfg{ "COM5", 19200, 8, modbus::Parity::Even, modbus::StopBits::One, 500 };
port.open(cfg);

modbus::RtuMaster master(port);
master.setTimeout(500);

// Read 2 holding registers from slave 1 starting at 0x0000:
std::vector<uint16_t> regs;
auto r = master.readHoldingRegisters(1, 0x0000, 2, regs);
if (r.ok()) {
    uint32_t v = (uint32_t(regs[0]) << 16) | regs[1];
    // ...
} else if (r.status == modbus::Status::ExceptionCode) {
    // device replied with exception code r.exceptionCode
} else {
    // r.message describes the failure
}

// Write single register:
master.writeSingleRegister(1, 0x0010, 1234);

// Inspect raw frames (for logging):
auto tx = master.lastTx();
auto rx = master.lastRx();
logArea->append((L"TX: " + modbus::toHex(tx) + L"\r\n").c_str());
logArea->append((L"RX: " + modbus::toHex(rx) + L"\r\n").c_str());
```

## Notes

- Synchronous / blocking — call from a worker thread (`CreateThread`) for long scans, not from `loop()`.
- CRC poly 0xA001, low-byte first, table-driven (fast).
- `setTimeout()` writes through to `ModbusSerialPort::readTimeoutMs` for the next call.
- For device discovery, sweep `slave` from 1..247 with a small request like `readHoldingRegisters(slave, probeAddr, 1, regs)` and treat any `Ok` or `ExceptionCode` response as "present".
