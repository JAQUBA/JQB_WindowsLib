#pragma once

#include <windows.h>

namespace TimerUtils {

// Restarts a one-shot style debounce timer by killing existing instance first.
void restartDebounceTimer(HWND hwnd, UINT_PTR timerId, UINT delayMs);

// Stops a timer if it is active.
void stopTimer(HWND hwnd, UINT_PTR timerId);

} // namespace TimerUtils
