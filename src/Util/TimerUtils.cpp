#include "Util/TimerUtils.h"

namespace TimerUtils {

void restartDebounceTimer(HWND hwnd, UINT_PTR timerId, UINT delayMs) {
    if (!hwnd) return;
    KillTimer(hwnd, timerId);
    SetTimer(hwnd, timerId, delayMs, NULL);
}

void stopTimer(HWND hwnd, UINT_PTR timerId) {
    if (!hwnd) return;
    KillTimer(hwnd, timerId);
}

} // namespace TimerUtils
