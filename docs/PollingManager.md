# PollingManager

`Util/PollingManager.h` — periodic polling groups ticked from `loop()`. Generic, no protocol dependency.

## Concept

Register named groups, each with an interval and an action. Call `tick()` once per `loop()` cycle. Any group whose interval has elapsed and which is `enabled` fires its `action`.

```cpp
struct Group {
    std::string           name;
    DWORD                 intervalMs = 1000;
    bool                  enabled    = false;
    DWORD                 nextTick   = 0;
    std::function<void()> action;
};
```

## API

```cpp
void   addGroup(const std::string& name, DWORD intervalMs, std::function<void()> action);
Group* find    (const std::string& name);
void   setEnabled (const std::string& name, bool enabled);
void   setInterval(const std::string& name, DWORD intervalMs);
void   trigger    (const std::string& name); // run action once now
void   tick();                               // call from loop()
```

`trigger()` invokes the action immediately (regardless of `enabled` or `nextTick`) — useful for "Refresh now" buttons.

## Example

```cpp
#include <Util/PollingManager.h>

PollingManager polling;

void setup() {
    polling.addGroup("monitor", 500, []() {
        readSensors();
    });
    polling.addGroup("status", 5000, []() {
        refreshStatusBar();
    });

    polling.setEnabled("monitor", true);
    polling.setEnabled("status",  true);
}

void loop() {
    polling.tick();
}

// "Refresh now" button:
button->onClick([&](Button*) { polling.trigger("monitor"); });

// Change cadence at runtime:
slider->onChange([&](int v) { polling.setInterval("monitor", (DWORD)v); });
```

## Notes

- Uses `GetTickCount()` (DWORD millisecond clock) — wraps every ~49 days.
- Single-threaded — runs actions on the UI thread. Keep them short or offload to a worker `CreateThread`.
- Adding a group while one is firing is safe; removal is not currently supported (recreate the manager).
