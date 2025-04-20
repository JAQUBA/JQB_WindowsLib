#ifndef __CORE_H__
#define __CORE_H__

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <list>
#include <queue>
#include <stdbool.h>
#include <cstdint>
#include <functional>

class Core {
    public:
        Core();
};

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