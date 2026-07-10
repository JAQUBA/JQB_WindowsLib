---
name: jqb-io-pipeline
description: "Implement robust IO pipelines in JQB_WindowsLib apps for Serial, BLE, HID, Modbus RTU, and Audio with init/connect sequence, callbacks, polling, and logging."
argument-hint: "Specify protocol, command flow, expected telemetry, and reconnect behavior"
user-invocable: true
---
# JQB IO Pipeline

Creates predictable communication pipelines for device-driven desktop tools.

## Use When
- Adding a protocol stack to an app.
- Refactoring unstable connection logic.
- Introducing monitoring and reconnect behavior.

## Procedure
1. Select protocol-specific stack:
   - Serial: IO/Serial
   - BLE: IO/BLE
   - HID: IO/HID
   - Modbus RTU: IO/Modbus/ModbusSerialPort + IO/Modbus/ModbusRTU
   - Audio: IO/Audio/AudioEngine
2. Apply [protocol decision matrix](./references/protocol-decision-matrix.md).
3. Implement init-before-connect lifecycle.
4. Add TX/RX logging and error channel with TextLogger.
5. Place periodic refresh into PollingManager group.
6. Move scans/sweeps to worker thread when runtime exceeds interactive latency.

## Validation
- Connect/disconnect cycles are repeatable.
- Reconnect path works after cable/power interruption.
- UI remains responsive during background operations.
