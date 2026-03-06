# ConfigManager — Configuration Manager

> `#include <Util/ConfigManager.h>`

## Description

Simple configuration manager saving settings in `key=value` format (INI-like file). Automatically loads configuration on creation and saves on destruction.

## Constructor

```cpp
ConfigManager(const std::string& configFilePath = "config.ini");
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `configFilePath` | `std::string` | Path to configuration file |

> Constructor automatically calls `loadFromFile()`.

## File Format

```ini
# Comment — lines starting with # or ; are ignored
key1=value1
key2=value2
port=COM3
baudrate=9600
```

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `setValue(key, value)` | `void` | Stores a key-value pair |
| `getValue(key, defaultValue)` | `std::string` | Gets value (or default) |
| `hasKey(key)` | `bool` | Checks if key exists |
| `removeKey(key)` | `void` | Removes a key |
| `saveToFile()` | `bool` | Saves to file |
| `loadFromFile()` | `bool` | Loads from file |
| `setConfigFilePath(path)` | `void` | Changes file path |
| `getKeys()` | `vector<string>` | List of all keys |

## Examples

### Basic Usage

```cpp
#include <Util/ConfigManager.h>

ConfigManager config("settings.ini");

// Write
config.setValue("port", "COM3");
config.setValue("baudrate", "9600");
config.setValue("username", "admin");

// Read
std::string port = config.getValue("port", "COM1");       // "COM3"
std::string baud = config.getValue("baudrate", "115200");  // "9600"
std::string missing = config.getValue("nonexistent", "default");  // "default"

// Check
if (config.hasKey("port")) {
    // key exists
}

// Remove
config.removeKey("username");

// Manual save (automatic on destructor)
config.saveToFile();
```

### Remembering Window Settings

```cpp
ConfigManager cfg("app_settings.ini");

void setup() {
    std::string lastPort = cfg.getValue("last_port", "COM1");
    std::string lastDevice = cfg.getValue("last_device", "");
    
    // ... use values to configure controls
}

// On settings change:
void onPortChange(const char* port) {
    cfg.setValue("last_port", port);
    // Auto-save on program exit (destructor)
}
```

### Listing Keys

```cpp
std::vector<std::string> keys = config.getKeys();
for (const auto& key : keys) {
    std::string value = config.getValue(key);
    // ...
}
```

## Lifecycle

```
ConfigManager("config.ini")
     │
     ▼
  loadFromFile()     ← automatic in constructor
     │
     ▼
  setValue() / getValue() ...  ← usage in program
     │
     ▼
  ~ConfigManager()
     │
     ▼
  saveToFile()       ← automatic in destructor
```

## Notes

- Whitespace (spaces, tabs) around keys and values is automatically trimmed
- Empty keys are ignored
- Comments: lines starting with `#` or `;`
- Empty lines are skipped
- File is fully rewritten on `saveToFile()` (does not update individual entries)
- No support for `[section]` — all keys in a flat namespace
- File encoding: system-dependent (usually ANSI/UTF-8)
