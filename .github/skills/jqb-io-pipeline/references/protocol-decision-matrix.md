# Protocol Decision Matrix

## Serial (COM)
Use when:
- Device exposes plain COM port.
- You need framed byte streams and simple request/response.

## Modbus RTU
Use when:
- Device map is register-based.
- You need parity/stop/data bits and strict timeout control.
- You need FC01/02/03/04/05/06/15/16 behavior.

## BLE
Use when:
- Device exposes GATT services and notify/write characteristics.
- Pairing and signal environment variability are expected.

## HID
Use when:
- Vendor/product IDs and feature reports are protocol contract.
- Driverless USB transport is required.

## Audio
Use when:
- Application generates or samples waveforms.
- Real-time preview and signal visualization are needed.

## Cross-cutting Rules
- Always call init() before operational calls.
- Emit structured INFO/ERROR/TX/RX logs.
- Keep parsing separate from transport callbacks.
