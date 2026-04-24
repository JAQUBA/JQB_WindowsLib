# TextLogger

`Util/TextLogger.h` — generic `TextArea` sink for `INFO` / `ERROR` / `TX` / `RX` messages. Filterable per-category, with a buffered snapshot for export.

## Why

Most desktop tools have a "log" tab. `TextLogger` provides a thin, reusable layer:

- attach to any existing `TextArea`
- tag each line as info / error / tx / rx
- hide a category at runtime via boolean fields
- get the full buffered text for "Save log…" actions

No protocol coupling — `tx` / `rx` are just labels for byte vectors with built-in hex formatting.

## API

```cpp
class TextLogger {
public:
    void attach(TextArea* sink);

    // category filters (live)
    bool showInfo  = true;
    bool showError = true;
    bool showTx    = true;
    bool showRx    = true;

    void info  (const std::wstring& msg);
    void error (const std::wstring& msg);
    void tx    (const wchar_t* tag, const std::vector<uint8_t>& bytes);
    void rx    (const wchar_t* tag, const std::vector<uint8_t>& bytes);

    std::wstring snapshot() const; // full buffer (for export)
    void         clear();
};
```

## Example

```cpp
#include <Util/TextLogger.h>
#include <UI/TextArea/TextArea.h>

TextArea*   logArea = new TextArea(0, 0, 600, 400);
window->add(logArea);
logArea->setFont(L"Consolas", 10);

TextLogger logger;
logger.attach(logArea);

logger.info(L"Aplikacja uruchomiona");
logger.error(L"Brak portu COM");
logger.tx(L"CMD", { 0x01, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC4, 0x0B });
logger.rx(L"OK",  { 0x01, 0x03, 0x04, 0x12, 0x34, 0x56, 0x78, 0xAA, 0xBB });

// Toggle categories from CheckBoxes:
checkboxTx->onChange([&](CheckBox*, bool c){ logger.showTx = c; });

// Export:
auto text = logger.snapshot(); // std::wstring with the whole log
```

## Output Format

Each line has a tag column followed by the message:

```
[INFO]  Aplikacja uruchomiona
[ERR ]  Brak portu COM
[TX ]   CMD: 01 03 00 00 00 02 C4 0B
[RX ]   OK : 01 03 04 12 34 56 78 AA BB
```

`tx` / `rx` automatically format the byte vector as space-separated uppercase hex.

## Notes

- Single-threaded — call from UI thread.
- The internal buffer grows unbounded; call `clear()` periodically for long-running sessions.
- Setting filters does not retroactively hide already-emitted lines; it only affects future writes.
