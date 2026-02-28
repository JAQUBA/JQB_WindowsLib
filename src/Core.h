#ifndef __CORE_H__
#define __CORE_H__

/* Ensure modern Windows API is available */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0600
#endif

#include <windows.h>

/* --- MinGW compatibility helpers --- */
#include <string>
#include <cwchar>

#ifndef __JQB_COMPAT_TO_WSTRING
#define __JQB_COMPAT_TO_WSTRING
namespace jqb_compat {
    inline std::wstring to_wstring(unsigned long val) {
        wchar_t buf[32];
        wsprintfW(buf, L"%lu", val);
        return std::wstring(buf);
    }
    inline std::wstring to_wstring(int val) {
        wchar_t buf[32];
        wsprintfW(buf, L"%d", val);
        return std::wstring(buf);
    }
    inline std::wstring to_wstring(long val) {
        wchar_t buf[32];
        wsprintfW(buf, L"%ld", val);
        return std::wstring(buf);
    }
}
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <list>
#include <queue>
#include <stdbool.h>
#include <cstdint>
#include <functional>

#define __weak __attribute__((weak))

class Core {
    public:
        HINSTANCE hInstance;
        HINSTANCE hPrevInstance;
        LPSTR lpCmdLine;
        int nCmdShow;
        Core();
};

extern Core _core;

#ifdef __cplusplus
extern "C" {
#endif

void init();
void setup();
void loop();

#ifdef __cplusplus
}
#endif

#endif // __CORE_H__