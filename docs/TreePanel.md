# TreePanel

LISTBOX-based checkable tree widget with collapsible sections.

## Header

```cpp
#include <UI/TreePanel/TreePanel.h>
```

## What It Is For

`TreePanel` is useful for side panels with many visibility toggles, especially:

- layer visibility in engineering/canvas apps
- grouped options with parent/child hierarchy
- action rows mixed with check rows

It is heavily used in real-world tools where a flat list of checkboxes does not scale.

## Core Concept

`TreePanel` does not own your data. It binds to external `bool` flags.

Typical flow:

1. Build tree with `clear()` + `add*()` calls.
2. On list click, call `handleClick(index)`.
3. If it returns `true`, rebuild tree from current state.

## Main API

- `TreePanel(HWND hListbox)`
- `void clear()`
- `void addSection(const wchar_t* name, bool* expanded)`
- `void addExpandGroup(const wchar_t* name, bool* expanded, int indentLevel = 1)`
- `void addItem(const wchar_t* name, bool* flag, int indentLevel = 1, std::function<void()> onToggle = nullptr)`
- `void addActionItem(const wchar_t* name, bool active, std::function<void()> onAction, int indentLevel = 1)`
- `bool handleClick(int listboxIndex)`
- `bool* getClickedFlag(int listboxIndex) const`

## Indentation Levels

- `0` section / top-level
- `1` child item
- `2` grandchild item
- `3` great-grandchild item

## Minimal Example

```cpp
#include <Core.h>
#include <windows.h>
#include <commctrl.h>
#include <UI/SimpleWindow/SimpleWindow.h>
#include <UI/TreePanel/TreePanel.h>

SimpleWindow* window = nullptr;
HWND hLayerList = NULL;
TreePanel* tree = nullptr;

bool secBoard = true;
bool secCopper = true;
bool secCopperSub = true;

bool visOutline = true;
bool visTop = true;
bool visBottom = false;
bool visTraces = true;
bool visPads = true;

void rebuildTree() {
    tree->clear();

    tree->addSection(L"Board", &secBoard);
    if (secBoard) {
        tree->addItem(L"Outline", &visOutline, 1);
    }

    tree->addSection(L"Copper", &secCopper);
    if (secCopper) {
        tree->addItem(L"Top", &visTop, 1);
        tree->addItem(L"Bottom", &visBottom, 1);

        tree->addExpandGroup(L"Top Sub-layers", &secCopperSub, 1);
        if (secCopperSub) {
            tree->addItem(L"Traces", &visTraces, 2);
            tree->addItem(L"Pads", &visPads, 2);
        }

        tree->addActionItem(L"Focus Copper", false, []() {
            visOutline = true;
            visTop = true;
            visBottom = true;
            visTraces = true;
            visPads = true;
        }, 1);
    }
}

static LRESULT CALLBACK AppSubclassProc(HWND hwnd, UINT msg, WPARAM wParam,
                                        LPARAM lParam, UINT_PTR, DWORD_PTR) {
    if (msg == WM_COMMAND && LOWORD(wParam) == 9500 && HIWORD(wParam) == LBN_SELCHANGE) {
        int idx = (int)SendMessageW(hLayerList, LB_GETCURSEL, 0, 0);
        if (tree && tree->handleClick(idx)) {
            rebuildTree();
        }
        return 0;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void setup() {
    window = new SimpleWindow(420, 500, "TreePanel Example", 0);
    window->init();

    hLayerList = CreateWindowExW(0, L"LISTBOX", NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_HASSTRINGS | LBS_NOINTEGRALHEIGHT,
        12, 12, 390, 440,
        window->getHandle(), (HMENU)(intptr_t)9500, _core.hInstance, NULL);

    tree = new TreePanel(hLayerList);
    rebuildTree();

    SetWindowSubclass(window->getHandle(), AppSubclassProc, 1, 0);

    window->onClose([]() {
        if (tree) {
            delete tree;
            tree = nullptr;
        }
    });
}

void loop() {}
```

## Notes

1. `handleClick()` deselects current list item intentionally; listbox acts as a click target.
2. `addActionItem()` does not manage state by itself; callback must update your flags.
3. If an `addItem()` callback (`onToggle`) is provided, it is called after the flag flips.
4. Keep one `rebuildTree()` function as a single source of UI truth.
