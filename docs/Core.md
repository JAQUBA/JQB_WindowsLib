# Core — Application Core

> `#include <Core.h>`

## Description

`Core` is the heart of JQB_WindowsLib. It provides:
- The Windows application entry point (`WinMain`)
- The message loop
- Arduino-like programming model: `init()` → `setup()` → `loop()`

## How It Works

```
1. Global object `_core` is created automatically
2. Core constructor calls init()
3. Windows calls WinMain()
4. WinMain calls setup(), then starts the message loop
5. loop() is called in each message loop cycle
6. WM_QUIT ends the loop and closes the application
```

## Class `Core`

```cpp
class Core {
public:
    HINSTANCE hInstance;       // Application instance handle
    HINSTANCE hPrevInstance;   // Previous instance handle (always NULL)
    LPSTR     lpCmdLine;      // Command line arguments
    int       nCmdShow;       // Window display mode (SW_SHOW, etc.)
    
    Core();                   // Constructor — calls init()
};

extern Core _core;            // Global object — accessible everywhere
```

## Callback Functions

All three functions are optional (marked `__weak`). If not defined, empty default implementations are used.

### `void init()`

Called in the `Core` constructor, **before** `WinMain`. Used for very early initialization.

```cpp
void init() {
    // e.g. configure global variables
}
```

### `void setup()`

Called once, at the beginning of `WinMain`, **before** the message loop. Ideal for:
- Creating windows
- Adding UI components
- Establishing connections (Serial, BLE)

```cpp
void setup() {
    SimpleWindow* window = new SimpleWindow(800, 600, "My App", 0);
    window->init();
    // ... add components
}
```

### `void loop()`

Called in each message loop cycle, **after** processing a Windows message.

```cpp
void loop() {
    // update state, poll sensors, etc.
}
```

> **Note:** `loop()` is called after each `DispatchMessage()`, not at fixed time intervals. Frequency depends on the number of messages in the queue.

## Message Loop (internals)

```cpp
int WINAPI WinMain(HINSTANCE hInstance, ...) {
    _core.hInstance = hInstance;
    // ...
    setup();
    
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_QUIT) break;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        loop();
    }
    return (int)msg.wParam;
}
```

## Accessing the Instance

The global object `_core` is accessible from any file:

```cpp
#include <Core.h>

void myFunction() {
    HINSTANCE hInst = _core.hInstance;  // Needed e.g. for CreateWindow
}
```

## Complete Application Example

```cpp
#include <Core.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/Label/Label.h>

SimpleWindow* window;
Label* label;
int counter = 0;

void init() {
    // Early initialization — optional
}

void setup() {
    window = new SimpleWindow(300, 200, "Counter App", 0);
    window->init();
    
    label = new Label(20, 20, 260, 30, L"Counter: 0");
    window->add(label);
}

void loop() {
    // NOTE: loop() depends on Windows messages
    // For periodic operations prefer SetTimer in WinAPI
}
```
