# Example Project 02 - Serial Monitor

Runnable serial monitor project with port list, connect/disconnect, RX log, and TX input.

## Build

```bash
pio run
```

## Run

```bash
pio run --target exec
```

## Notes

- Requires available COM ports on the machine.
- Uses `Serial::updateComPorts()` and `Serial::getAvailablePorts()`.
- Uses `TextArea` as the live log output.
