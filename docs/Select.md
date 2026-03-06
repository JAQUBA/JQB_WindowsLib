# Select — Dropdown List (ComboBox)

> `#include <UI/Select/Select.h>`

## Description

Dropdown selection list (ComboBox). Supports:
- Adding items statically and dynamically
- `onChange` callback on selection change
- Linking with an external vector (`link()`)
- UTF-8 text with automatic Unicode conversion

## Constructor

```cpp
Select(int x, int y, int width, int height, const char* text,
       std::function<void(Select*)> onChange);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `x`, `y` | `int` | Position |
| `width`, `height` | `int` | Size (height = edit field height) |
| `text` | `const char*` | Initial text (UTF-8) |
| `onChange` | `function<void(Select*)>` | Callback invoked on selection change |

## Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `create(HWND parent)` | `void` | Creates the ComboBox control |
| `addItem(const char* item)` | `void` | Adds an item to the list |
| `setText(const char* text)` | `void` | Sets the selected text |
| `clear()` | `void` | Clears all items |
| `link(const vector<string>* items)` | `void` | Links with an external vector |
| `updateItems()` | `void` | Refreshes the list from the linked vector |
| `handleSelection()` | `void` | Handles selection change (internal) |
| `getText()` | `const char*` | Text of currently selected item |
| `getHandle()` | `HWND` | Control handle |
| `getId()` | `int` | Unique ID (auto from 3000) |

## Examples

### List with Static Items

```cpp
Select* sel = new Select(20, 50, 200, 25, "Select port",
    [](Select* s) {
        const char* selected = s->getText();
        // React to selection
    }
);
sel->addItem("COM1");
sel->addItem("COM3");
sel->addItem("COM5");
window->add(sel);
```

### Linking with External Vector

```cpp
Serial serial;
serial.init();

Select* selPort = new Select(20, 50, 200, 25, "COM Ports",
    [&serial](Select* s) {
        serial.setPort(s->getText());
    }
);
selPort->link(&serial.getAvailablePorts());
window->add(selPort);

// When the port list changes:
serial.updateComPorts();
selPort->updateItems();   // Refreshes control data from the linked vector
```

### Programmatic Selection

```cpp
sel->setText("COM3");  // Selects the "COM3" item
```

## Notes

- IDs start at **3000**
- `height` refers to the edit field. The dropdown list is automatically taller (+150 px)
- `link()` does not copy data — it holds a pointer to the vector. The vector must outlive the Select
- `clear()` resets the list and `selectedIndex`
