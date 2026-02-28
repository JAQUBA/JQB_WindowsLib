# ConfigManager — Menedżer konfiguracji

> `#include <Util/ConfigManager.h>`

## Opis

Prosty menedżer konfiguracji zapisujący ustawienia w formacie `klucz=wartość` (plik INI-like). Automatycznie wczytuje konfigurację przy tworzeniu i zapisuje przy zniszczeniu obiektu.

## Konstruktor

```cpp
ConfigManager(const std::string& configFilePath = "config.ini");
```

| Parametr | Typ | Opis |
|----------|-----|------|
| `configFilePath` | `std::string` | Ścieżka do pliku konfiguracyjnego |

> Konstruktor automatycznie wywołuje `loadFromFile()`.

## Format pliku

```ini
# Komentarz — linie zaczynające się od # lub ; są ignorowane
klucz1=wartość1
klucz2=wartość2
port=COM3
baudrate=9600
```

## Metody

| Metoda | Zwraca | Opis |
|--------|--------|------|
| `setValue(key, value)` | `void` | Zapisuje parę klucz-wartość |
| `getValue(key, defaultValue)` | `std::string` | Pobiera wartość (lub domyślną) |
| `hasKey(key)` | `bool` | Sprawdza istnienie klucza |
| `removeKey(key)` | `void` | Usuwa klucz |
| `saveToFile()` | `bool` | Zapisuje do pliku |
| `loadFromFile()` | `bool` | Wczytuje z pliku |
| `setConfigFilePath(path)` | `void` | Zmienia ścieżkę pliku |
| `getKeys()` | `vector<string>` | Lista wszystkich kluczy |

## Przykłady

### Podstawowe użycie

```cpp
#include <Util/ConfigManager.h>

ConfigManager config("settings.ini");

// Zapis
config.setValue("port", "COM3");
config.setValue("baudrate", "9600");
config.setValue("username", "admin");

// Odczyt
std::string port = config.getValue("port", "COM1");       // "COM3"
std::string baud = config.getValue("baudrate", "115200");  // "9600"
std::string missing = config.getValue("nonexistent", "default");  // "default"

// Sprawdzenie
if (config.hasKey("port")) {
    // klucz istnieje
}

// Usunięcie
config.removeKey("username");

// Manual save (automatyczny przy destruktorze)
config.saveToFile();
```

### Zapamiętywanie ustawień okna

```cpp
ConfigManager cfg("app_settings.ini");

void setup() {
    std::string lastPort = cfg.getValue("last_port", "COM1");
    std::string lastDevice = cfg.getValue("last_device", "");
    
    // ... użyj wartości do ustawienia kontrolek
}

// Przy zmianie ustawień:
void onPortChange(const char* port) {
    cfg.setValue("last_port", port);
    // Auto-save przy wyjściu z programu (destruktor)
}
```

### Lista kluczy

```cpp
std::vector<std::string> keys = config.getKeys();
for (const auto& key : keys) {
    std::string value = config.getValue(key);
    // ...
}
```

## Cykl życia

```
ConfigManager("config.ini")
     │
     ▼
  loadFromFile()     ← automatycznie w konstruktorze
     │
     ▼
  setValue() / getValue() ...  ← użycie w programie
     │
     ▼
  ~ConfigManager()
     │
     ▼
  saveToFile()       ← automatycznie w destruktorze
```

## Uwagi

- Białe znaki (spacje, tabulatory) wokół kluczy i wartości są automatycznie usuwane (trimming)
- Puste klucze są ignorowane
- Komentarze: linie zaczynające się od `#` lub `;`
- Puste linie są pomijane
- Plik nadpisywany w całości przy `saveToFile()` (nie aktualizuje pojedynczych wpisów)
- Brak obsługi sekcji `[section]` — wszystkie klucze w jednej płaskiej przestrzeni
- Kodowanie pliku: zależne od systemu (zwykle ANSI/UTF-8)
