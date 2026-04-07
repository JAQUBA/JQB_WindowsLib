# TimerUtils

Lightweight WinAPI timer helpers for debounce-style UI workflows.

## Header

```cpp
#include <Util/TimerUtils.h>
```

## API

```cpp
namespace TimerUtils {
    void restartDebounceTimer(HWND hwnd, UINT_PTR timerId, UINT delayMs);
    void stopTimer(HWND hwnd, UINT_PTR timerId);
}
```

- `restartDebounceTimer(...)`: `KillTimer` + `SetTimer` for debounce behavior.
- `stopTimer(...)`: safe helper that stops timer only when window handle is valid.

## Example

```cpp
static const UINT_PTR TIMER_ID = 9601;

void onInputChanged(HWND hwnd) {
    TimerUtils::restartDebounceTimer(hwnd, TIMER_ID, 400);
}

LRESULT onWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM) {
    if (msg == WM_TIMER && wParam == TIMER_ID) {
        TimerUtils::stopTimer(hwnd, TIMER_ID);
        // perform debounced work
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, 0);
}
```
