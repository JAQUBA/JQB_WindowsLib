# TCP — TCP Client

> `#include <IO/TCP/TCP.h>`

## Description

Minimal TCP client module with:
- Connection via hostname or IP address (resolved with `getaddrinfo`)
- Threaded data reception (non-blocking)
- Callbacks: `onConnect`, `onDisconnect`, `onReceive`, `onError`
- `TCP_NODELAY` enabled by default (low-latency, suited for transparent bridging)

## Important: Include Order

`TCP.h` sets `WIN32_LEAN_AND_MEAN` and includes `<winsock2.h>`/`<ws2tcpip.h>` **before** `Core.h` (which pulls in `<windows.h>`). To avoid `winsock.h`/`winsock2.h` redefinition clashes, include `IO/TCP/TCP.h` **before** any other JQB_WindowsLib header in a translation unit:

```cpp
#include <IO/TCP/TCP.h>   // must come first
#include <Core.h>
#include <IO/Serial/Serial.h>
```

## Constructor

```cpp
TCP();
```

No parameters required. Connection parameters are provided to `connect()`.

## Methods

### Connection

| Method | Returns | Description |
|--------|---------|--------------|
| `connect(const std::string& host, unsigned short port)` | `bool` | Resolves `host` (IP or hostname) and connects. Starts read thread on success |
| `disconnect()` | `void` | Shuts down and closes the socket, stops the read thread |
| `isConnected()` | `bool` | Whether connection is active |

### Sending and Receiving

| Method | Returns | Description |
|--------|---------|--------------|
| `write(const std::vector<uint8_t>& data)` | `bool` | Sends data (loops until fully flushed) |
| `send(const std::vector<uint8_t>& data)` | `bool` | Alias for `write()` |

### Callbacks

| Method | Callback | Description |
|--------|----------|--------------|
| `onConnect(function<void()>)` | Connected | Called after successful `connect()` |
| `onDisconnect(function<void()>)` | Disconnected | Called on `disconnect()` (only if previously connected) |
| `onReceive(function<void(const vector<uint8_t>&)>)` | Data received | Called from the read thread |
| `onError(function<void()>)` | Connection error | Called from the read thread when the connection is lost unexpectedly |

### Connection Loss Detection

| Method | Returns | Description |
|--------|---------|--------------|
| `isConnectionLost()` | `bool` | Whether the connection was lost unexpectedly (check from `loop()`) |
| `clearConnectionLost()` | `void` | Resets the flag after handling it |

## Examples

### Basic Communication

```cpp
#include <IO/TCP/TCP.h>

TCP tcp;

void setup() {
    tcp.onConnect([]() {
        // Connected
    });

    tcp.onDisconnect([]() {
        // Disconnected
    });

    tcp.onReceive([](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        // ... handle received bytes
    });

    tcp.onError([]() {
        // Connection lost unexpectedly
    });

    tcp.connect("127.0.0.1", 12345);
}

void loop() {
    if (tcp.isConnectionLost()) {
        tcp.clearConnectionLost();
        // ... update UI, disconnect the other side of a bridge, etc.
    }
}
```

### Sending Data

```cpp
std::vector<uint8_t> packet = {0x55, 0xAA, 0x01, 0x00};
tcp.write(packet);

// Or using send()
tcp.send(packet);
```

## Read Thread

The read thread runs in the background after `connect()`:
1. Blocks on `recv()` waiting for data
2. On data received → invokes `onReceive` callback
3. On graceful close (`recv()` returns 0) or socket error → signals connection lost, invokes `onError`, thread exits
4. On explicit `disconnect()` → `shutdown()` unblocks `recv()`, thread exits cleanly without invoking `onError`
5. Stopped on `disconnect()` or destructor

## Notes

- `WSAStartup`/`WSACleanup` are reference-counted across all `TCP` instances in the process — safe to create multiple instances
- `TCP_NODELAY` is always enabled on the connected socket
- No automatic reconnect — call `connect()` again after a disconnect/error
- Address resolution supports both IPv4/IPv6 literals and hostnames via `getaddrinfo` (`AF_UNSPEC`)
