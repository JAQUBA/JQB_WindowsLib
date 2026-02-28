# HID — USB HID Communication

Klasa `HID` zapewnia komunikację z urządzeniami USB HID przez **Feature Reports**.  
Dynamicznie ładuje `hid.dll` — nie wymaga linkowania statycznego biblioteki HID.

**Include:** `#include <IO/HID/HID.h>`

---

## Klasa `HID`

### Konstruktor

```cpp
HID hid;   // domyślny — brak konfiguracji
```

### Konfiguracja (przed `findAndOpen()`)

| Metoda | Opis |
|--------|------|
| `setVidPid(uint16_t vid, uint16_t pid)` | VID/PID urządzenia docelowego |
| `setUsage(uint16_t usagePage, uint16_t usage)` | Usage Page i Usage do filtrowania interfejsu |
| `setFeatureReportSize(size_t dataBytes)` | Rozmiar danych Feature Report w bajtach (bez Report ID). Domyślnie: **7** |

### Cykl życia

```
HID hid;
hid.init();                              // ładuje hid.dll
hid.setVidPid(0x1209, 0xC55D);          // VID/PID
hid.setUsage(0xFF00, 0x01);             // Vendor Usage Page
hid.setFeatureReportSize(7);            // 7 bajtów danych
if (hid.findAndOpen()) {
    // komunikacja...
    hid.close();
}
```

---

## Metody

### `bool init()`

Ładuje `hid.dll` i rozwiązuje wskaźniki funkcji. Musi być wywołane raz przed innymi metodami.

**Zwraca:** `true` jeśli hid.dll załadowane poprawnie.

---

### `bool findAndOpen()`

Enumeruje urządzenia HID (przez SetupDI) i otwiera **pierwsze** pasujące do ustawionych VID/PID i Usage.

**Zwraca:** `true` jeśli urządzenie znalezione i otwarte.

---

### `std::vector<HIDDevice> scan()`

Zwraca listę wszystkich urządzeń HID pasujących do VID/PID i Usage **bez otwierania** ich.

```cpp
auto devices = hid.scan();
for (auto& dev : devices) {
    printf("Found: %s\n", dev.toString().c_str());
}
```

---

### `bool open(const std::string& devicePath)`

Otwiera konkretne urządzenie po ścieżce (z `HIDDevice::path`).

```cpp
auto devices = hid.scan();
if (!devices.empty()) {
    hid.open(devices[0].path);
}
```

---

### `void close()`

Zamyka uchwyt urządzenia. Wywołuje callback `onDisconnect`.

---

### `bool isOpen() const`

Czy urządzenie jest otwarte.

---

### `bool getFeatureReport(uint8_t reportId, uint8_t* data, size_t dataLen)`

Odczytuje Feature Report z urządzenia.

| Parametr | Opis |
|----------|------|
| `reportId` | HID Report ID |
| `data` | Bufor wyjściowy (minimum `dataLen` bajtów) |
| `dataLen` | Liczba bajtów danych do odczytania |

**Zwraca:** `true` przy sukcesie.

---

### `bool setFeatureReport(uint8_t reportId, const uint8_t* data, size_t dataLen)`

Wysyła Feature Report do urządzenia.

| Parametr | Opis |
|----------|------|
| `reportId` | HID Report ID |
| `data` | Dane do wysłania |
| `dataLen` | Liczba bajtów danych |

**Zwraca:** `true` przy sukcesie.

---

## Callbacki

| Metoda | Sygnatura | Kiedy |
|--------|-----------|-------|
| `onConnect(cb)` | `void()` | Po pomyślnym otwarciu urządzenia |
| `onDisconnect(cb)` | `void()` | Po zamknięciu urządzenia |
| `onError(cb)` | `void(const std::string&)` | Przy błędzie (ładowanie dll, komunikacja) |

```cpp
hid.onConnect([]() {
    // urządzenie otwarte
});

hid.onError([](const std::string& msg) {
    printf("HID Error: %s\n", msg.c_str());
});
```

---

## Struct `HIDDevice`

Informacje o znalezionym urządzeniu (zwracane przez `scan()`):

| Pole | Typ | Opis |
|------|-----|------|
| `vendorId` | `uint16_t` | VID |
| `productId` | `uint16_t` | PID |
| `versionNumber` | `uint16_t` | Numer wersji urządzenia |
| `usagePage` | `uint16_t` | HID Usage Page |
| `usage` | `uint16_t` | HID Usage |
| `path` | `std::string` | Ścieżka interfejsu (do `open()`) |

Metoda `toString()` zwraca opis w formie: `"HID 1209:C55D  UsagePage=0xFF00 Usage=0x0001"`

---

## Kompletny przykład

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

    lblStatus = new Label(20, 20, 360, 25, L"Rozłączony");
    window->add(lblStatus);

    hid.init();
    hid.setVidPid(MY_VID, MY_PID);
    hid.setUsage(USAGE_PAGE, USAGE_ID);
    hid.setFeatureReportSize(REPORT_SIZE);

    hid.onConnect([]() {
        lblStatus->setText(L"Połączony!");
    });

    hid.onDisconnect([]() {
        lblStatus->setText(L"Rozłączony");
    });

    hid.onError([](const std::string& msg) {
        // obsługa błędów
    });

    window->add(new Button(20, 60, 120, 30, "Połącz", [](Button*) {
        if (hid.isOpen()) {
            hid.close();
        } else {
            hid.findAndOpen();
        }
    }));

    window->add(new Button(150, 60, 120, 30, "Odczytaj", [](Button*) {
        if (!hid.isOpen()) return;
        uint8_t data[REPORT_SIZE];
        if (hid.getFeatureReport(REPORT_ID_CFG, data, REPORT_SIZE)) {
            // przetwarzanie danych...
        }
    }));
}

void loop() {}
```

---

## Uwagi techniczne

- **hid.dll** jest ładowane dynamicznie (`LoadLibrary`) — nie wymaga `-lhid` w linkowania
- **setupapi** jest wymagane (linkowane automatycznie przez bibliotekę)
- Urządzenie otwierane jest z `FILE_SHARE_READ | FILE_SHARE_WRITE` (współdzielony dostęp — klawiatury są urządzeniami systemowymi)
- `setVidPid(0, 0)` + `setUsage(0, 0)` = brak filtra — `scan()` zwróci **wszystkie** urządzenia HID
- Feature Report buffer = `[ReportID][data...]` — klasa automatycznie dodaje/usuwa bajt Report ID
