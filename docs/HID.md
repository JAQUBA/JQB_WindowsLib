# HID — USB HID Communication

The `HID` class provides communication with USB HID devices via **Feature Reports**.  
Dynamically loads `hid.dll` — no static linking of the HID library required.

**Include:** `#include <IO/HID/HID.h>`

---

## Class `HID`

### Constructor

```cpp
HID hid;   // default — no configuration
```

### Configuration (before `findAndOpen()`)

| Method | Description |
|--------|-------------|
| `setVidPid(uint16_t vid, uint16_t pid)` | Target device VID/PID |
| `setUsage(uint16_t usagePage, uint16_t usage)` | Usage Page and Usage for interface filtering |
| `setFeatureReportSize(size_t dataBytes)` | Feature Report data size in bytes (excluding Report ID). Default: **7** |

### Lifecycle

```
HID hid;
hid.init();                              // loads hid.dll
hid.setVidPid(0x1209, 0xC55D);          // VID/PID
hid.setUsage(0xFF00, 0x01);             // Vendor Usage Page
hid.setFeatureReportSize(7);            // 7 data bytes
if (hid.findAndOpen()) {
    // communication...
    hid.close();
}
```

---

## Methods

### `bool init()`

Loads `hid.dll` and resolves function pointers. Must be called once before other methods.

**Returns:** `true` if hid.dll loaded successfully.

---

### `bool findAndOpen()`

Enumerates HID devices (via SetupDI) and opens the **first** one matching the configured VID/PID and Usage.

**Returns:** `true` if device found and opened.

---

### `std::vector<HIDDevice> scan()`

Returns a list of all HID devices matching VID/PID and Usage **without opening** them.

```cpp
auto devices = hid.scan();
for (auto& dev : devices) {
    printf("Found: %s\n", dev.toString().c_str());
}
```

---

### `bool open(const std::string& devicePath)`

Opens a specific device by path (from `HIDDevice::path`).

```cpp
auto devices = hid.scan();
if (!devices.empty()) {
    hid.open(devices[0].path);
}
```

---

### `void close()`

Closes the device handle. Invokes `onDisconnect` callback.

---

### `bool isOpen() const`

Whether a device is currently open.

---

### `bool getFeatureReport(uint8_t reportId, uint8_t* data, size_t dataLen)`

Reads a Feature Report from the device.

| Parameter | Description |
|-----------|-------------|
| `reportId` | HID Report ID |
| `data` | Output buffer (minimum `dataLen` bytes) |
| `dataLen` | Number of data bytes to read |

**Returns:** `true` on success.

---

### `bool setFeatureReport(uint8_t reportId, const uint8_t* data, size_t dataLen)`

Sends a Feature Report to the device.

| Parameter | Description |
|-----------|-------------|
| `reportId` | HID Report ID |
| `data` | Data to send |
| `dataLen` | Number of data bytes |

**Returns:** `true` on success.

---

## Callbacks

| Method | Signature | When |
|--------|-----------|------|
| `onConnect(cb)` | `void()` | After successful device open |
| `onDisconnect(cb)` | `void()` | After device close |
| `onError(cb)` | `void(const std::string&)` | On error (dll loading, communication) |

```cpp
hid.onConnect([]() {
    // device opened
});

hid.onError([](const std::string& msg) {
    printf("HID Error: %s\n", msg.c_str());
});
```

---

## Struct `HIDDevice`

Information about a found device (returned by `scan()`):

| Field | Type | Description |
|-------|------|-------------|
| `vendorId` | `uint16_t` | VID |
| `productId` | `uint16_t` | PID |
| `versionNumber` | `uint16_t` | Device version number |
| `usagePage` | `uint16_t` | HID Usage Page |
| `usage` | `uint16_t` | HID Usage |
| `path` | `std::string` | Interface path (for `open()`) |

Method `toString()` returns a description: `"HID 1209:C55D  UsagePage=0xFF00 Usage=0x0001"`

---

## Complete Example

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Button/Button.h>
#include <UI/Label/Label.h>
#include <IO/HID/HID.h>

#define MY_VID          0x1209
#define MY_PID          0xC55D
#define USAGE_PAGE      0xFF00
#define USAGE_ID        0x01
#define REPORT_SIZE     7
#define REPORT_ID_CFG   2

SimpleWindow* window;
Label* lblStatus;
HID hid;

void setup() {
    window = new SimpleWindow(400, 200, "HID Demo", 0);
    window->init();

    lblStatus = new Label(20, 20, 360, 25, L"Disconnected");
    window->add(lblStatus);

    hid.init();
    hid.setVidPid(MY_VID, MY_PID);
    hid.setUsage(USAGE_PAGE, USAGE_ID);
    hid.setFeatureReportSize(REPORT_SIZE);

    hid.onConnect([]() {
        lblStatus->setText(L"Connected!");
    });

    hid.onDisconnect([]() {
        lblStatus->setText(L"Disconnected");
    });

    hid.onError([](const std::string& msg) {
        // error handling
    });

    window->add(new Button(20, 60, 120, 30, "Connect", [](Button*) {
        if (hid.isOpen()) {
            hid.close();
        } else {
            hid.findAndOpen();
        }
    }));

    window->add(new Button(150, 60, 120, 30, "Read", [](Button*) {
        if (!hid.isOpen()) return;
        uint8_t data[REPORT_SIZE];
        if (hid.getFeatureReport(REPORT_ID_CFG, data, REPORT_SIZE)) {
            // process data...
        }
    }));
}

void loop() {}
```

---

## Technical Notes

- **hid.dll** is loaded dynamically (`LoadLibrary`) — does not require `-lhid` in linking
- **setupapi** is required (linked automatically by the library)
- Device is opened with `FILE_SHARE_READ | FILE_SHARE_WRITE` (shared access — keyboards are system devices)
- `setVidPid(0, 0)` + `setUsage(0, 0)` = no filter — `scan()` will return **all** HID devices
- Feature Report buffer = `[ReportID][data...]` — the class automatically adds/removes the Report ID byte
