# Serial — Komunikacja przez port COM

> `#include <IO/Serial/Serial.h>`

## Opis

Moduł komunikacji przez port szeregowy (COM / RS-232) z:
- Automatycznym wykrywaniem portów COM (przez SetupAPI)
- Wątkowym odbiorem danych (non-blocking)
- Callbackami: `onConnect`, `onDisconnect`, `onReceive`
- Auto-reconnect po błędach komunikacyjnych

## Konstruktor

```cpp
Serial();
```

Nie wymaga parametrów. Parametry połączenia konfigurowane są wewnętrznie.

## Domyślne parametry połączenia

| Parametr | Wartość |
|----------|---------|
| Baud Rate | 9600 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| DTR | Enabled |
| RTS | Enabled |

## Metody

### Inicjalizacja i połączenie

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `init()` | `bool` | Inicjalizuje moduł — skanuje dostępne porty COM |
| `connect()` | `bool` | Otwiera port i uruchamia wątek odczytu |
| `disconnect()` | `void` | Zamyka port i zatrzymuje wątek |
| `isConnected()` | `bool` | Czy połączenie jest aktywne |
| `setPort(const char* portName)` | `void` | Ustawia port (np. `"COM3"`) |
| `updateComPorts()` | `void` | Ponownie skanuje porty |
| `getAvailablePorts()` | `const vector<string>&` | Lista dostępnych portów |

### Wysyłanie i odbieranie

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `write(const vector<uint8_t>& data)` | `bool` | Wysyła dane |
| `send(const vector<uint8_t>& data)` | `bool` | Alias dla `write()` |
| `read(vector<uint8_t>& data, size_t n)` | `bool` | Odczytuje `n` bajtów (synchronicznie) |

### Callbacki

| Metoda | Callback | Opis |
|--------|----------|------|
| `onConnect(function<void()>)` | Połączono | Wywoływany po udanym `connect()` |
| `onDisconnect(function<void()>)` | Rozłączono | Wywoływany przy `disconnect()` |
| `onReceive(function<void(const vector<uint8_t>&)>)` | Odebrano dane | Wywoływany z wątku odczytu |

## Przykłady

### Podstawowa komunikacja

```cpp
#include <IO/Serial/Serial.h>

Serial serial;

void setup() {
    serial.init();
    
    // Lista portów
    for (const auto& port : serial.getAvailablePorts()) {
        // np. "COM3", "COM5"
    }
    
    serial.setPort("COM3");
    serial.connect();
}
```

### Z callbackami

```cpp
Serial serial;

void setup() {
    serial.init();
    
    serial.onConnect([]() {
        lblStatus->setText(L"Połączono!");
    });
    
    serial.onDisconnect([]() {
        lblStatus->setText(L"Rozłączono");
    });
    
    serial.onReceive([](const std::vector<uint8_t>& data) {
        std::string text(data.begin(), data.end());
        textLog->append(text);
    });
    
    serial.setPort("COM3");
    serial.connect();
}
```

### Integracja z Select

```cpp
Serial serial;
serial.init();

Select* selPort = new Select(20, 50, 200, 25, "Port",
    [](Select* s) {
        serial.setPort(s->getText());
    }
);

// Użyj link() do automatycznej synchronizacji
selPort->link(&serial.getAvailablePorts());
window->add(selPort);

// Aby odświeżyć listę portów:
serial.updateComPorts();
selPort->updateItems();
```

### Wysyłanie danych

```cpp
std::vector<uint8_t> packet = {0x55, 0xAA, 0x01, 0x00};
serial.write(packet);

// Lub za pomocą send()
serial.send(packet);
```

## Wątek odczytu

Wątek odczytu działa w tle po `connect()`:
1. Co 10 ms sprawdza `ClearCommError()` czy są dane
2. Jeśli tak → `ReadFile()` i wywołanie `onReceive` callback
3. Po kilku kolejnych błędach → auto-reconnect (disconnect + connect)
4. Zatrzymywany przy `disconnect()` lub destruktorze

## Uwagi

- **Enumeracja portów:** Użyty `SetupDiGetClassDevsA` z `GUID_DEVCLASS_PORTS`. Pobiera „Friendly Name" (np. `"USB Serial Port (COM3)"`) i wyciąga numer portu
- Nazwy portów: `"COM1"`, `"COM3"` itd. — bez `\\.\` (prefiks dodawany wewnętrznie)
- **Timeouty:** ReadInterval=50, ReadTotal=50+10×bytes, WriteTotal=50+10×bytes
- Bufor odczytu: do 256 bajtów na cykl
- `maxConsecutiveErrors = 10` → auto-reconnect po 10 błędach z rzędu
